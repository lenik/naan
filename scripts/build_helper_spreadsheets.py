#!/usr/bin/env python3
# Copyright (C) 2026 Lenik <naan@bodz.net>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Build helper.ods and helper.xlsx for naan spreadsheet use.

Requires a running LibreOffice listening on the UNO socket, e.g.:
  soffice --headless --accept="socket,host=127.0.0.1,port=2002;urp;"
"""

from __future__ import annotations

import hashlib
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_ODS = ROOT / "helper.ods"
OUT_XLSX = ROOT / "helper.xlsx"
OUT_BAS = ROOT / "helper-naan.bas"
UNO_URL = os.environ.get(
    "NAAN_UNO_URL",
    "uno:socket,host=127.0.0.1,port=2002;urp;StarOffice.ComponentContext",
)

SAMPLES = [
    ("hello", "SHA256", 65536, 0),
    ("hello", "MD5", 65536, 0),
    ("hello", "SHA1", 2000, 0),
    ("hello", "SHA256", 256, 0),
    ("hello", "SHA256", 4294967296, 0),
    ("world", "SHA256", 65536, 0),
    ("service-name", "SHA1", 2000, 0),
]
BLANK_ROWS = 13

# LibreOffice Basic UDF: prefer ScriptForge HashStr; else python3 hashlib via shell.
LO_BASIC = r'''Option Explicit

' naan helper: Number = (digest(Name) mod Modulus) + Bias
' Hash via Python hashlib (MD5 SHA1 SHA224 SHA256 SHA384 SHA512).
' Requires macros enabled and /usr/bin/python3.

Private Function NaanNormalizeAlgo(ByVal Algorithm As String) As String
    Dim a As String
    a = UCase(Trim(Algorithm))
    a = Replace(a, "-", "")
    a = Replace(a, "_", "")
    If a = "" Then a = "SHA256"
    If a = "SHA2" Then a = "SHA256"
    NaanNormalizeAlgo = a
End Function

Private Function NaanAppendByte(ByVal out As String, ByVal b As Integer) As String
    If out <> "" Then out = out & ","
    NaanAppendByte = out & CStr(b And 255)
End Function

Private Function NaanUtf8Bytes(ByVal s As String) As String
    Dim i As Long
    Dim cp As Long
    Dim out As String
    out = ""
    For i = 1 To Len(s)
        cp = Asc(Mid(s, i, 1))
        If cp < 128 Then
            out = NaanAppendByte(out, cp)
        ElseIf cp < 2048 Then
            out = NaanAppendByte(out, 192 + (cp \ 64))
            out = NaanAppendByte(out, 128 + (cp Mod 64))
        ElseIf cp < 65536 Then
            out = NaanAppendByte(out, 224 + (cp \ 4096))
            out = NaanAppendByte(out, 128 + ((cp \ 64) Mod 64))
            out = NaanAppendByte(out, 128 + (cp Mod 64))
        Else
            out = NaanAppendByte(out, 240 + (cp \ 262144))
            out = NaanAppendByte(out, 128 + ((cp \ 4096) Mod 64))
            out = NaanAppendByte(out, 128 + ((cp \ 64) Mod 64))
            out = NaanAppendByte(out, 128 + (cp Mod 64))
        End If
    Next i
    NaanUtf8Bytes = out
End Function

Function NAAN(Name As String, Optional Algorithm As String, Optional Modulus As Double, Optional Bias As Double) As Variant
    Dim algo As String
    Dim m As Double
    Dim biasVal As Double
    Dim iCh As Integer
    Dim line As String
    Dim pyFile As String
    Dim outFile As String
    Dim shFile As String
    Dim home As String
    On Error GoTo Fail

    If Name = "" Then
        NAAN = ""
        Exit Function
    End If
    If IsMissing(Algorithm) Then
        algo = "SHA256"
    ElseIf Algorithm = "" Then
        algo = "SHA256"
    Else
        algo = Algorithm
    End If
    If IsMissing(Modulus) Then
        m = 65536
    Else
        m = Modulus
    End If
    If IsMissing(Bias) Then
        biasVal = 0
    Else
        biasVal = Bias
    End If
    If m <= 0 Then
        NAAN = CVErr(510)
        Exit Function
    End If

    algo = NaanNormalizeAlgo(algo)
    home = Environ("HOME")
    If home = "" Then home = "/tmp"
    On Error Resume Next
    MkDir home & "/.cache"
    On Error GoTo Fail

    pyFile = home & "/.cache/naan-helper-run.py"
    outFile = home & "/.cache/naan-helper-out.txt"
    shFile = home & "/.cache/naan-helper-run.sh"

    iCh = Freefile
    Open pyFile For Output As iCh
    Print #iCh, "import hashlib"
    Print #iCh, "algo = '" & algo & "'.lower()"
    Print #iCh, "name = bytes([" & NaanUtf8Bytes(Name) & "])"
    Print #iCh, "mod = int(" & Trim(Str(m)) & ")"
    Print #iCh, "bias = int(" & Trim(Str(biasVal)) & ")"
    Print #iCh, "h = hashlib.new(algo, name).digest()"
    Print #iCh, "acc = 0"
    Print #iCh, "for x in h:"
    Print #iCh, "    acc = (acc * 256 + x) % mod"
    Print #iCh, "print(acc + bias)"
    Close #iCh

    iCh = Freefile
    Open shFile For Output As iCh
    Print #iCh, "#!/bin/sh"
    Print #iCh, "/usr/bin/python3 '" & pyFile & "' > '" & outFile & "'"
    Close #iCh

    Shell "/bin/sh", 0, shFile, True

    iCh = Freefile
    Open outFile For Input As iCh
    Line Input #iCh, line
    Close #iCh
    NAAN = CDbl(Trim(line))
    Exit Function
Fail:
    NAAN = CVErr(502)
End Function
'''

EXCEL_VBA = r'''Option Explicit

' naan helper for Excel: Number = (digest(Name) mod Modulus) + Bias
' Algorithms: MD5 SHA1 SHA256 SHA384 SHA512 (.NET). For SHA224 use helper.ods.

Private Function NaanNormalizeAlgo(ByVal Algorithm As String) As String
    Dim a As String
    a = UCase$(Trim$(Algorithm))
    a = Replace(a, "-", "")
    a = Replace(a, "_", "")
    If a = "" Then a = "SHA256"
    If a = "SHA2" Then a = "SHA256"
    NaanNormalizeAlgo = a
End Function

Private Function NaanDigestHex(ByVal Name As String, ByVal Algorithm As String) As String
    Dim algo As String
    Dim enc As Object
    Dim hasher As Object
    Dim bytes() As Byte
    Dim hash() As Byte
    Dim i As Long
    Dim hexOut As String

    algo = NaanNormalizeAlgo(Algorithm)
    Set enc = CreateObject("System.Text.UTF8Encoding")
    bytes = enc.GetBytes_4(Name)

    Select Case algo
        Case "MD5"
            Set hasher = CreateObject("System.Security.Cryptography.MD5CryptoServiceProvider")
        Case "SHA1"
            Set hasher = CreateObject("System.Security.Cryptography.SHA1Managed")
        Case "SHA256"
            Set hasher = CreateObject("System.Security.Cryptography.SHA256Managed")
        Case "SHA384"
            Set hasher = CreateObject("System.Security.Cryptography.SHA384Managed")
        Case "SHA512"
            Set hasher = CreateObject("System.Security.Cryptography.SHA512Managed")
        Case Else
            Err.Raise 5, , "Unsupported algorithm: " & Algorithm
    End Select

    hash = hasher.ComputeHash_2((bytes))
    hexOut = ""
    For i = LBound(hash) To UBound(hash)
        hexOut = hexOut & LCase$(Right$("0" & Hex$(hash(i)), 2))
    Next i
    NaanDigestHex = hexOut
End Function

Private Function NaanReduce(ByVal DigestHex As String, ByVal Modulus As Double) As Double
    Dim i As Long
    Dim acc As Double
    Dim b As Double
    Dim pair As String
    If Modulus <= 0 Then
        NaanReduce = 0
        Exit Function
    End If
    acc = 0
    For i = 1 To Len(DigestHex) Step 2
        pair = Mid$(DigestHex, i, 2)
        If Len(pair) = 1 Then pair = pair & "0"
        b = CDbl("&H" & pair)
        acc = acc * 256# + b
        acc = acc - Int(acc / Modulus) * Modulus
    Next i
    NaanReduce = acc
End Function

Public Function NAAN(Optional Name As Variant, Optional Algorithm As Variant, Optional Modulus As Variant, Optional Bias As Variant) As Variant
    Dim n As String
    Dim algo As String
    Dim m As Double
    Dim biasVal As Double
    Dim digest As String
    On Error GoTo Fail
    If IsMissing(Name) Or IsEmpty(Name) Or Name = "" Then
        NAAN = ""
        Exit Function
    End If
    n = CStr(Name)
    If IsMissing(Algorithm) Or IsEmpty(Algorithm) Then
        algo = "SHA256"
    Else
        algo = CStr(Algorithm)
    End If
    If IsMissing(Modulus) Or IsEmpty(Modulus) Then
        m = 65536
    Else
        m = CDbl(Modulus)
    End If
    If IsMissing(Bias) Or IsEmpty(Bias) Then
        biasVal = 0
    Else
        biasVal = CDbl(Bias)
    End If
    If m <= 0 Then
        NAAN = CVErr(xlErrNum)
        Exit Function
    End If
    digest = NaanDigestHex(n, algo)
    NAAN = NaanReduce(digest, m) + biasVal
    Exit Function
Fail:
    NAAN = CVErr(xlErrValue)
End Function
'''


def naan_number(name: str, algo: str, mod: int, bias: int) -> int:
    a = algo.lower().replace("-", "").replace("_", "")
    if a == "sha2":
        a = "sha256"
    digest = getattr(hashlib, a)(name.encode("utf-8")).digest()
    acc = 0
    for b in digest:
        acc = (acc * 256 + b) % mod
    return acc + bias


def build_ods() -> None:
    import uno
    from com.sun.star.beans import PropertyValue

    local = uno.getComponentContext()
    resolver = local.ServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", local
    )
    ctx = resolver.resolve(UNO_URL)
    sm = ctx.ServiceManager
    desktop = sm.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)

    if OUT_ODS.exists():
        OUT_ODS.unlink()

    doc = desktop.loadComponentFromURL("private:factory/scalc", "_blank", 0, ())
    lc = doc.BasicLibraries
    if not lc.hasByName("Standard"):
        lc.createLibrary("Standard")
    if not lc.isLibraryLoaded("Standard"):
        lc.loadLibrary("Standard")
    lib = lc.getByName("Standard")
    if lib.hasByName("Naan"):
        lib.replaceByName("Naan", LO_BASIC)
    else:
        lib.insertByName("Naan", LO_BASIC)

    # Remove default Sheet1 rename
    sheets = doc.Sheets
    sheet = sheets.getByIndex(0)
    sheet.Name = "naan"

    headers = ["Name", "Algorithm", "Modulus", "Bias", "Number"]
    for i, h in enumerate(headers):
        sheet.getCellByPosition(i, 0).String = h
        sheet.getCellByPosition(i, 0).CharWeight = 150

    for r, (name, algo, mod, bias) in enumerate(SAMPLES, start=1):
        sheet.getCellByPosition(0, r).String = name
        sheet.getCellByPosition(1, r).String = algo
        sheet.getCellByPosition(2, r).Value = float(mod)
        sheet.getCellByPosition(3, r).Value = float(bias)
        sheet.getCellByPosition(4, r).Formula = f"=NAAN(A{r + 1};B{r + 1};C{r + 1};D{r + 1})"

    start_blank = 1 + len(SAMPLES)
    for r in range(start_blank, start_blank + BLANK_ROWS):
        sheet.getCellByPosition(4, r).Formula = f"=NAAN(A{r + 1};B{r + 1};C{r + 1};D{r + 1})"

    if sheets.hasByName("README"):
        sheets.removeByName("README")
    sheets.insertNewByName("README", 1)
    readme = sheets.getByName("README")
    notes = [
        "naan spreadsheet helper",
        "",
        "Number = (digest(Name) mod Modulus) + Bias  — same as the naan CLI.",
        "Enable macros when opening. Number uses the NAAN() UDF (Standard.Naan).",
        "",
        "Hash algorithms (Python hashlib; same set ScriptForge documents):",
        "  MD5, SHA1, SHA224, SHA256, SHA384, SHA512",
        "Algorithm names are case-insensitive; hyphens optional (sha-256 == SHA256).",
        "Defaults when omitted: Algorithm=SHA256, Modulus=65536, Bias=0 (profile w).",
        "",
        "Profiles (for reference):",
        "  b   SHA256  Modulus=256           Bias=0",
        "  w   SHA256  Modulus=65536         Bias=0  (default)",
        "  dw  SHA256  Modulus=4294967296    Bias=0",
        "  wm  SHA1    Modulus=2000          Bias=0",
        "",
        "LibreOffice NAAN() shells to /usr/bin/python3 + hashlib (cache files under",
        "~/.cache/naan-helper-*). Enable macros when opening.",
        "",
        "Excel: see helper.xlsx (VBA sheet) or helper-naan.bas.",
    ]
    for i, line in enumerate(notes):
        readme.getCellByPosition(0, i).String = line

    url = uno.systemPathToFileUrl(str(OUT_ODS))
    doc.storeAsURL(
        url,
        (
            PropertyValue(Name="FilterName", Value="calc8"),
            PropertyValue(Name="Overwrite", Value=True),
        ),
    )
    doc.close(True)

    # Reopen, recalculate, verify
    props = (
        PropertyValue(Name="Hidden", Value=True),
        PropertyValue(Name="MacroExecutionMode", Value=4),
    )
    doc = desktop.loadComponentFromURL(url, "_blank", 0, props)
    sheet = doc.Sheets.getByName("naan")
    doc.calculateAll()
    failed = []
    for r, (name, algo, mod, bias) in enumerate(SAMPLES, start=1):
        exp = naan_number(name, algo, mod, bias)
        cell = sheet.getCellByPosition(4, r)
        got = int(cell.Value) if cell.Error == 0 else None
        if got != exp:
            failed.append((r + 1, got, exp, cell.String, cell.Error))
    doc.close(True)
    if failed:
        for item in failed:
            print("VERIFY FAIL", item, file=sys.stderr)
        raise SystemExit("helper.ods NAAN results do not match naan")


def build_xlsx() -> None:
    import openpyxl
    from openpyxl.comments import Comment
    from openpyxl.styles import Font
    from openpyxl.utils import get_column_letter

    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = "naan"
    ws.append(["Name", "Algorithm", "Modulus", "Bias", "Number"])
    for cell in ws[1]:
        cell.font = Font(bold=True)

    for i, (name, algo, mod, bias) in enumerate(SAMPLES, start=2):
        n = naan_number(name, algo, mod, bias)
        ws.cell(i, 1, name)
        ws.cell(i, 2, algo)
        ws.cell(i, 3, mod)
        ws.cell(i, 4, bias)
        c = ws.cell(i, 5)
        c.value = f"=NAAN(A{i},B{i},C{i},D{i})"
        c.comment = Comment(f"naan expected: {n}", "naan")

    start_blank = 2 + len(SAMPLES)
    for i in range(start_blank, start_blank + BLANK_ROWS):
        ws.cell(i, 5).value = f"=NAAN(A{i},B{i},C{i},D{i})"

    for col, width in enumerate([18, 12, 14, 10, 14], start=1):
        ws.column_dimensions[get_column_letter(col)].width = width

    ws2 = wb.create_sheet("README")
    for i, line in enumerate(
        [
            "naan spreadsheet helper (Excel)",
            "",
            "Number = (digest(Name) mod Modulus) + Bias — same as the naan CLI.",
            "",
            "Excel has no built-in MD5/SHA worksheet functions. To make Number work:",
            "  1. Alt+F11 → Insert → Module",
            "  2. Paste the code from the VBA sheet (or helper-naan.bas)",
            "  3. Save as .xlsm if you want macros kept",
            "",
            "Or open helper.ods in LibreOffice (enable macros).",
            "",
            "Supported Algorithm values: MD5, SHA1, SHA256, SHA384, SHA512",
            "Defaults: Algorithm=SHA256, Modulus=65536, Bias=0",
            "",
            "Sample Number cells have comments with the expected naan CLI result.",
        ],
        start=1,
    ):
        ws2.cell(i, 1, line)
    ws2.column_dimensions["A"].width = 100

    ws3 = wb.create_sheet("VBA")
    for i, line in enumerate(EXCEL_VBA.splitlines(), start=1):
        ws3.cell(i, 1, line)
    ws3.column_dimensions["A"].width = 100

    wb.save(OUT_XLSX)
    OUT_BAS.write_text(EXCEL_VBA + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> None:
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--ods-only", action="store_true", help="build helper.ods only (needs UNO)"
    )
    parser.add_argument(
        "--xlsx-only", action="store_true", help="build helper.xlsx only"
    )
    args = parser.parse_args(argv)

    for name, algo, mod, bias in SAMPLES:
        print(f"{name!r} {algo} mod={mod} bias={bias} => {naan_number(name, algo, mod, bias)}")

    do_xlsx = not args.ods_only
    do_ods = not args.xlsx_only
    if do_xlsx:
        build_xlsx()
        print(f"Wrote {OUT_XLSX} ({OUT_XLSX.stat().st_size} bytes)")
        print(f"Wrote {OUT_BAS} ({OUT_BAS.stat().st_size} bytes)")
    if do_ods:
        build_ods()
        print(f"Wrote {OUT_ODS} ({OUT_ODS.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
