Option Explicit

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

