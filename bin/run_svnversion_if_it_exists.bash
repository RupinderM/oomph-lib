#! /bin/bash

#======================================================
# Run svnversion if it exists; return revision number. 
# If we don't have snversion or if we're working with 
# an unversioned version return 0000 as the dummy 
# revision number
#======================================================
path_to_svnversion=`which svnversion`
if [ "$path_to_svnversion" == ""  ] 
then
   #Dummy return
   echo "0000"
else
   revision=`$path_to_svnversion`
   if [ "$revision" == "Unversioned directory" ]
   then
       echo "0000"
   else
       echo $revision
   fi
fi