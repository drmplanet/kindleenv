if [ -f base3rd.tgz ];then
echo " "
else
echo "0. get patch zip file"
cat patch/base3rd.tgz* > base3rd.tgz

echo "1. patching............"
cd koreader
tar zxvf ../base3rd.tgz
fi



echo "2. build koreader.zip"
./kodev release kindle

