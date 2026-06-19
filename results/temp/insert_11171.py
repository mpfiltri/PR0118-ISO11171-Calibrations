import xlwings as xw
import glob, os, re

# ---- configuration --------------------------------------------------------
TEMPLATE = r".\LPA3 NAVAIR TEMPLATE.xlsx"   # template workbook
FOLDER   = r"."                              # folder containing the 20 destination files
SHEET    = "11171"                           # sheet to copy across
DRY_RUN  = False                             # True = process 1 file, don't save; False = run all and save
# ---------------------------------------------------------------------------

TEMPLATE_BASENAME = os.path.basename(TEMPLATE)
# matches '[LPA3 NAVAIR TEMPLATE.xlsx]SheetName'!  or  [LPA3 NAVAIR TEMPLATE.xlsx]SheetName!
ext = re.compile(r"'?\[" + re.escape(TEMPLATE_BASENAME) + r"\]([^'!]+)'?!", re.IGNORECASE)

def _fix(m):
    """Re-quote sheet names that contain spaces or non-alphanumeric chars."""
    sheet = m.group(1)
    if re.search(r"[^A-Za-z0-9_]", sheet):
        return f"'{sheet}'!"
    return f"{sheet}!"

app = xw.App(visible=False)
app.display_alerts = False
app.screen_updating = False
try:
    tpl = app.books.open(os.path.abspath(TEMPLATE), read_only=True)

    targets = [f for f in glob.glob(os.path.join(FOLDER, "*LPA3 NAVAIR*.xlsx"))
               if not os.path.basename(f).startswith("~$")
               and os.path.abspath(f) != os.path.abspath(TEMPLATE)]
    targets.sort()
    if DRY_RUN:
        targets = targets[:1]

    print(f"{'DRY RUN — ' if DRY_RUN else ''}{len(targets)} file(s) to process\n")

    for path in targets:
        name = os.path.basename(path)
        wb = app.books.open(os.path.abspath(path))

        # remove a stale copy so re-runs are idempotent
        for s in list(wb.sheets):
            if s.name == SHEET:
                s.delete()

        # copy the template sheet to the END of this workbook
        tpl.sheets[SHEET].api.Copy(Before=None, After=wb.sheets[-1].api)
        sht = wb.sheets[SHEET]

        # rewrite any external refs back to internal (same-named sheet in THIS file)
        rng = sht.used_range
        formulas = rng.formula            # 2-D tuple of formulas/values
        new_block, changed = [], False
        for row in formulas:
            new_row = []
            for f in row:
                if isinstance(f, str) and TEMPLATE_BASENAME.lower() in f.lower():
                    new_row.append(ext.sub(_fix, f)); changed = True
                else:
                    new_row.append(f)
            new_block.append(new_row)
        if changed:
            rng.formula = new_block

        # break any workbook-level external links the copy dragged in
        # (source of the "broken link to a record spreadsheet" warning)
        links = wb.api.LinkSources(1)   # 1 = xlExcelLinks
        if links:
            for link in links:
                wb.api.BreakLink(link, 1)
            print(f"  broke {len(links)} external link(s): {name}")

        # safety: flag any broken references
        bad = [c.address for c in rng
               if isinstance(c.value, str) and c.value.startswith("#REF!")]
        if bad:
            print(f"  !! {name}: #REF! in {bad}")

        if DRY_RUN:
            c19 = sht.range("C19").formula
            c3  = sht.range("C3").formula
            print(f"  C3  = {c3}")
            print(f"  C19 = {c19}")
            print(f"DRY RUN — not saving: {name}"
                  f"{'  (refs rewritten)' if changed else '  (refs already internal)'}")
        else:
            wb.save()
            print(f"saved: {name}"
                  f"{'  (refs rewritten)' if changed else ''}")
        wb.close()

    tpl.close()
    print("\ndone.")
finally:
    app.quit()
