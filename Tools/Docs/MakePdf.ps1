<#
Builds the story PDFs from docs/story/ with headless Microsoft Edge:
  - "Squirrel Wheels - Story.pdf"        from StoryOverview.html (the proofreading version)
  - "Squirrel Wheels - Voice Script.pdf" from VoiceScript.html, filled with lines from RadioScript.md

Run it from anywhere (Windows PowerShell 5.1 is enough):
    powershell -ExecutionPolicy Bypass -File D:\Games-Unreal\ProjectI\Tools\Docs\MakePdf.ps1 [-OutDir <folder>]

The PDFs go to your Downloads folder unless -OutDir says otherwise. The voice script takes every table row
between the "voice-script: start" and "voice-script: end" markers in RadioScript.md, groups the rows by voice
(D, B, G, N, U) in file order, and puts each voice's rows where the template has its LINES marker comment;
{{COUNT:X}}, {{TOTAL}} and {{DATE}} get filled in too. Rows that share an ID number (D-16a, D-16b) are variants
of one moment and print under the first row's "When". Edit lines in RadioScript.md, never in the PDF.
#>
param(
    [string]$OutDir = (Join-Path $env:USERPROFILE 'Downloads')
)

$ErrorActionPreference = 'Stop'
$storyDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..\docs\story')).Path
$workDir = Join-Path $env:TEMP 'SquirrelWheelsPdf'
New-Item -ItemType Directory -Force $workDir | Out-Null
New-Item -ItemType Directory -Force $OutDir | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
$voices = [ordered]@{ D = 'Dispatch'; B = 'Foreman Buck'; G = 'Gravy Train'; N = 'Night Owl'; U = 'Unit Seven' }

# Markdown cell -> HTML: escape, then **bold**, *italic*, and "quotes" as curly quotes
# (single ones inside a spoken line, which the page wraps in double quotes).
function ConvertTo-CellHtml([string]$Text, [switch]$Spoken) {
    $html = [System.Net.WebUtility]::HtmlEncode($Text.Trim())
    $html = $html -replace '\*\*(.+?)\*\*', '<b>$1</b>' -replace '\*(.+?)\*', '<i>$1</i>' -replace '`(.+?)`', '$1'
    if ($Spoken) { return $html -replace '&quot;(.+?)&quot;', '&lsquo;$1&rsquo;' }
    return $html -replace '&quot;(.+?)&quot;', '&ldquo;$1&rdquo;'
}

# Read the voice-script rows: | ID | Who | When | Line | Direction |
$rows = New-Object System.Collections.Generic.List[object]
$inScript = $false
$scene = ''
foreach ($line in [System.IO.File]::ReadAllLines((Join-Path $storyDir 'RadioScript.md'), $utf8)) {
    if ($line -match '<!--\s*voice-script:\s*start\s*-->') { $inScript = $true; continue }
    if ($line -match '<!--\s*voice-script:\s*end\s*-->') { $inScript = $false; continue }
    if (-not $inScript) { continue }
    if ($line -match '^###\s+(.+)$') { $scene = $Matches[1].Trim(); continue }
    $id = [regex]::Match($line, '^\|\s*(([A-Z])-(\d+))([a-z]?)\s*\|')
    if (-not $id.Success) { continue }
    $cells = @($line.Trim().Trim('|').Split('|') | ForEach-Object { $_.Trim() })
    if ($cells.Count -ne 5) { throw "RadioScript.md: expected 5 cells, got $($cells.Count): $line" }
    $voice = $id.Groups[2].Value
    if (-not $voices.Contains($voice)) { throw "RadioScript.md: unknown voice letter '$voice': $line" }
    $rows.Add([pscustomobject]@{
        Voice = $voice; Moment = $id.Groups[1].Value; Id = $cells[0]; Scene = $scene
        When = $cells[2]; Line = $cells[3]; Direction = $cells[4]
    })
}
if ($rows.Count -eq 0) { throw 'RadioScript.md: no rows between the voice-script markers.' }
$duplicates = @($rows | Group-Object Id | Where-Object Count -gt 1 | ForEach-Object Name)
if ($duplicates.Count) { throw "RadioScript.md: duplicate IDs: $($duplicates -join ', ')" }

# Fill the template.
$page = [System.IO.File]::ReadAllText((Join-Path $storyDir 'VoiceScript.html'), $utf8)
foreach ($voice in $voices.Keys) {
    $mine = @($rows | Where-Object Voice -eq $voice)
    $html = New-Object System.Text.StringBuilder
    $moment = $null
    foreach ($row in $mine) {
        if ($row.Moment -ne $moment) {
            if ($moment) { [void]$html.AppendLine('</div>') }
            [void]$html.AppendLine("<div class=""moment""><p class=""when""><span class=""scene"">$(ConvertTo-CellHtml $row.Scene)</span>$(ConvertTo-CellHtml $row.When)</p>")
            $moment = $row.Moment
        }
        [void]$html.AppendLine("<div class=""take""><span class=""id"">$($row.Id)</span><span class=""q"">&ldquo;$(ConvertTo-CellHtml $row.Line -Spoken)&rdquo;</span><span class=""dir"">$(ConvertTo-CellHtml $row.Direction)</span></div>")
    }
    if ($moment) { [void]$html.AppendLine('</div>') }
    $page = $page.Replace("<!--LINES:$voice-->", $html.ToString()).Replace("{{COUNT:$voice}}", "$($mine.Count)")
    if ($mine.Count) { Write-Host ('{0,-13} {1,3} lines' -f $voices[$voice], $mine.Count) }
}
$page = $page.Replace('{{TOTAL}}', "$($rows.Count)").Replace('{{DATE}}', (Get-Date).ToString('d MMMM yyyy', [cultureinfo]'en-GB'))
$voiceHtml = Join-Path $workDir 'VoiceScript.html'
[System.IO.File]::WriteAllText($voiceHtml, $page, $utf8)
Write-Host ('{0,-13} {1,3} lines' -f 'Total', $rows.Count)

# Print with headless Edge (its own profile folder, so an open Edge window doesn't get in the way).
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $edge) { throw 'Microsoft Edge not found.' }

function Write-Pdf([string]$Html, [string]$Name) {
    $pdf = Join-Path $OutDir $Name
    if (Test-Path $pdf) { Remove-Item $pdf }
    $url = ([System.Uri]$Html).AbsoluteUri
    $arguments = "--headless --disable-gpu --no-pdf-header-footer --user-data-dir=""$workDir\edge"" --print-to-pdf=""$pdf"" ""$url"""
    # Wait for Edge itself only: its helper processes can outlive it.
    $process = Start-Process -FilePath $edge -ArgumentList $arguments -PassThru -WindowStyle Hidden
    [void]$process.WaitForExit(60000)
    for ($i = 0; $i -lt 30 -and -not (Test-Path $pdf); $i++) { Start-Sleep -Milliseconds 500 }
    if (-not (Test-Path $pdf)) { throw "Edge didn't write $pdf" }
    Write-Host ('Wrote {0} ({1} KB)' -f $pdf, [math]::Round((Get-Item $pdf).Length / 1KB))
}

Write-Pdf (Join-Path $storyDir 'StoryOverview.html') 'Squirrel Wheels - Story.pdf'
Write-Pdf $voiceHtml 'Squirrel Wheels - Voice Script.pdf'
