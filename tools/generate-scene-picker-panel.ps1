# Generate a GP-safe scene picker panel (types 3/4/5 only — no knobs/GPSCALE).
# Output: docs/panels/PreSonusScenePicker.gppanel (UTF-8, no BOM)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path $PSScriptRoot -Parent
$out = Join-Path $repoRoot "docs\panels\PreSonusScenePicker.gppanel"

function New-RbWidget {
    param(
        [string]$Type,
        [string]$X, [string]$Y, [string]$W, [string]$H,
        [string]$Caption,
        [string]$HidePres,
        [string]$CapVisible,
        [string]$WidgetValue,
        [string]$Handle,
        [string]$BackPlate = "11",
        [string]$BackPlateVisible = "1",
        [string]$Extra = ""
    )
    return @"
  <WIDGET type="$Type" style="0" x="$X" y="$Y"
          width="$W" height="$H" caption="$Caption"
          captionIncludesAttribute="0" hideTemporaryValueDisplay="1"
          hideInPresentationView="$HidePres" overrideName="1"
          captionVisible="$CapVisible" captionHeight="0.25"
          reflectMIDI="" reflectMIDIOn="0" midiThru="0" midiSource="" midiDevice=""
          midiData="0" rigId="0" widgetValue="$WidgetValue" backPlate="$BackPlate"
          backPlateVisible="$BackPlateVisible" invertMIDI="0" displayRange="0"
          ignoresVariations="0" followHardware="0" relativeMode="0"
          enableDirectedOSC="0" directedOSCAddressPattern=""
          directedOSCIPAddress="" directedOSCPort="9001" directedOSCTargetName=""
          widgetDirectedOSCType="0" widgetDirectedMinIntValue="0"
          widgetDirectedMaxIntValue="127" widgetDirectedMinDoubleValue="0.0"
          widgetDirectedMaxDoubleValue="1.0" widgetDirectedStringListValue=""
          widgetHandle="$Handle" enableOSC="0" invertOSC="0" groupName=""
          widgetMoveBehavior="0" updateOnLoadGig="0" updateOnActivateRackspace="0"
          setDefaultDoubleClickResetValue="0.0" bckgColor="ff3c3c3c"
          outlineColor="ff808080" outlineRound="4" outlineThick="2"$Extra>
  </WIDGET>
"@
}

function New-RbLabel($X,$Y,$W,$H,$Cap,$Hide,$Handle,$Font) {
    $extra = " fontSize=""$Font"" textPos=""17"" textColor=""ffffffff"" usesDimming=""1.0"""
    New-RbWidget -Type 4 -X $X -Y $Y -W $W -H $H -Caption $Cap -HidePres $Hide `
        -CapVisible 1 -WidgetValue 1.0 -Handle $Handle -BackPlate 10 -BackPlateVisible 0 -Extra $extra
}

function New-RbButton($X,$Y,$W,$H,$Cap,$Hide,$Handle) {
    $extra = ' btnMomToLatch="1" btnPadLike="0"'
    New-RbWidget -Type 3 -X $X -Y $Y -W $W -H $H -Caption $Cap -HidePres $Hide `
        -CapVisible 1 -WidgetValue 0.0 -Handle $Handle -Extra $extra
}

function New-RbBackground {
    return @"
  <WIDGET type="5" style="0" x="0.01" y="0.01"
          width="0.98" height="0.98" caption=""
          captionIncludesAttribute="0" hideTemporaryValueDisplay="1"
          hideInPresentationView="0" overrideName="1"
          captionVisible="0" captionHeight="0.25"
          reflectMIDI="" reflectMIDIOn="0" midiThru="0" midiSource="" midiDevice=""
          midiData="0" rigId="0" widgetValue="1.0" backPlate="10"
          backPlateVisible="1" invertMIDI="0" displayRange="0"
          ignoresVariations="0" followHardware="0" relativeMode="0"
          enableDirectedOSC="0" directedOSCAddressPattern=""
          directedOSCIPAddress="" directedOSCPort="9001" directedOSCTargetName=""
          widgetDirectedOSCType="0" widgetDirectedMinIntValue="0"
          widgetDirectedMaxIntValue="127" widgetDirectedMinDoubleValue="0.0"
          widgetDirectedMaxDoubleValue="1.0" widgetDirectedStringListValue=""
          widgetHandle="" enableOSC="0" invertOSC="0" groupName=""
          widgetMoveBehavior="0" updateOnLoadGig="0" updateOnActivateRackspace="0"
          setDefaultDoubleClickResetValue="0.0" bckgColor="3f636363"
          outlineColor="fff5f5f5" outlineRound="5" outlineThick="1">
  </WIDGET>
"@
}

$xml = @"
<RACKUNIT size="4" style="0" color="ff000000" ver="4.099999904632568">
$(New-RbBackground)
$(New-RbLabel 0.02 0.06 0.60 0.16 Scene 0 CurrentSceneLabel 18.0)
$(New-RbButton 0.64 0.05 0.32 0.18 Select 0 SelectButton)
$(New-RbLabel 0.02 0.28 0.96 0.10 Project 1 PickerProjectLabel 14.0)
$(New-RbButton 0.02 0.40 0.10 0.12 '&lt;' 1 ProjPrev)
$(New-RbButton 0.14 0.40 0.10 0.12 '&gt;' 1 ProjNext)
$(New-RbLabel 0.78 0.40 0.20 0.12 '1/1' 1 ProjCountLabel 12.0)
$(New-RbLabel 0.02 0.54 0.96 0.10 Scene 1 PickerSceneLabel 14.0)
$(New-RbButton 0.02 0.66 0.10 0.12 '&lt;' 1 ScenePrev)
$(New-RbButton 0.14 0.66 0.10 0.12 '&gt;' 1 SceneNext)
$(New-RbLabel 0.78 0.66 0.20 0.12 '1/1' 1 SceneCountLabel 12.0)
$(New-RbButton 0.02 0.82 0.46 0.14 Recall 1 RecallButton)
$(New-RbButton 0.52 0.82 0.46 0.14 Done 1 DoneButton)
</RACKUNIT>
"@

[System.IO.File]::WriteAllText($out, $xml, (New-Object System.Text.UTF8Encoding $false))
$b = [System.IO.File]::ReadAllBytes($out)[0..2]
Write-Host "Wrote $out ($((Get-Item $out).Length) bytes, first-bytes=$($b -join ','))"
