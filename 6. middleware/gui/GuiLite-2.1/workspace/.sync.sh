if [ "$#" -ne 1 ]; then
    echo "Invalid arguments"
    exit -1
fi

url="https://api.powerbi.com/beta/72f988bf-86f1-41af-91ab-2d7cd011db47/datasets/2ff1e8a8-2f6f-4d73-a75d-86829e3f4574/rows?key=8f5xLp1gP8%2FzSee4vCUBcyjR65I9zZ6nb%2B%2F7bbzex%2FSctLX3ntIlAR0sxWpDdguuYyDtLdHK%2Fxbxj%2FrSBkX7eQ%3D%3D"
build_time=`date +%Y-%m-%dT%H:%M:%S.000%z`
device_info=`uname -s -n -m`

#--------------- Geo info ----------------#
if [ ! -f "/tmp/ip_info.tmp" ]; then
    curl ipinfo.io > /tmp/ip_info.tmp # get IP info
fi

if [ ! -f "/tmp/ip_city.tmp" ]; then
    grep city /tmp/ip_info.tmp > /tmp/ip_city.tmp # filter city
    sed -i 's/"city"://g' /tmp/ip_city.tmp #remove property name
    sed -i 's/"//g' /tmp/ip_city.tmp #remove double quotes
fi
city=`sed 's/,//g' /tmp/ip_city.tmp` #remove comma

if [ ! -f "/tmp/ip_country.tmp" ]; then
    grep country /tmp/ip_info.tmp > /tmp/ip_country.tmp # filter country
    sed -i 's/"country"://g' /tmp/ip_country.tmp #remove property name
    sed -i 's/"//g' /tmp/ip_country.tmp #remove double quotes
fi
country=`sed 's/,//g' /tmp/ip_country.tmp` #remove comma

if [ ! -f "/tmp/ip_org.tmp" ]; then
    grep org /tmp/ip_info.tmp > /tmp/ip_org.tmp # filter org
    sed -i 's/"org"://g' /tmp/ip_org.tmp #remove property name
    sed -i 's/"//g' /tmp/ip_org.tmp #remove double quotes
fi
org=`sed 's/,//g' /tmp/ip_org.tmp` #remove comma

curl --include --request POST --header "Content-Type: application/json" --data-binary "[{
\"device_info\" :\"$device_info\",
\"project_info\" :\"$1\",
\"time\" :\"$build_time\",
\"weight\" :1,
\"country\" :\"$country\",
\"city\" :\"$city\",
\"org\" :\"$org\",
\"log\" :\"$build_time\",
\"version\" :\"v2.1\"
}]" $url > /dev/null
