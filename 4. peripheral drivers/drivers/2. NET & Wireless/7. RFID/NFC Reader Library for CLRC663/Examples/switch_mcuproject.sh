#!/bin/bash
#
# (c) NXP Semiconductors
#  _   _            _   _  __     _____     _ _ _ _
# | \ | |          | \ | |/ _|   |  __ \   | | (_) |
# |  \| |_  ___ __ |  \| | |_ ___| |__) |__| | |_| |__
# | . ` \ \/ / '_ \| . ` |  _/ __|  _  // _` | | | '_ \
# | |\  |>  <| |_) | |\  | || (__| | \ \ (_| | | | |_) |
# |_| \_/_/\_\ .__/|_| \_|_| \___|_|  \_\__,_|_|_|_.__/
#            | |
#            |_|
#
# Helper Script to switch MCU Type for examples
#
# e.g. To switch Examples and Compliance APPs to PN7462AU:
#
#       cd Examples
#       ./switch_mcuproject.sh _PN74xxxx_cproject
#
#       cd ../ComplianceApp
#       ../Examples/switch_mcuproject.sh ../Examples/_PN74xxxx_cproject
#

if [ -z $1 ]
then
    echo "Usage $0 _Template_Project"
    exit
fi

if [ ! -f $1 ]
then
    echo "'$1' does not exist."
    exit
fi

for i in Nfcrdlib*;
do
    if [ "$i" = "Nfcrdlib_NFC_Forum_DTA_ComplApp" ]; then
	    echo "Updating Project File: $i "
        case $1 in
        ../Examples/_FRDMK82_cproject) cp _FRDMK82_DTA_cproject $i/mcux/.cproject;;
        ../Examples/_LPC1769_cproject) cp _LPC1769_DTA_cproject $i/mcux/.cproject;;
        ../Examples/_PN74xxxx_cproject) cp _PN74xxxx_DTA_cproject $i/mcux/.cproject;;
        esac
    elif [ "$i" = "NfcrdlibTst12_Rc663Lpcd" ]; then
        continue
    else
	    echo "Updating Project File: $i "
        sed s/THE_PROJECT_NAME/${i}_mcux/g $1 > $i/mcux/.cproject
    fi

#Updating the Heap and Stack options for Nfcrdlib_ISO10373_6_PCD_ComplApp compliance application in .cproject file
	if [ "$i" = "Nfcrdlib_ISO10373_6_PCD_ComplApp" ]; then
	   sed s/'Data;0x1090'/'Data;0xe00'/g $1 > $i/mcux/.cproject
	fi

done



