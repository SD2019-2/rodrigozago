# Commands

##Connects to gcloud with ssh
gcloud beta compute --project "sistemas-distribuidos-inf-ufg" ssh --zone "us-central1-a" "xdr-vm"

##Changes to repository folder
cd rodrigooliveira/xdr_json/

##Run server on vm
python3 server.py


##Run client on my PC
go run client.go

##Send messages

##Go to mysql console
mysql -u root -p
pass: sd-2019-2

##Select the database
use log;

##See the columns
select * from log;
