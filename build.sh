if [ -f base3rd.tgz ] || [ -f _patched_ ];then
echo " "
else

echo "1. patching............"
cd koreader
cat ../patch/base3rd.tgz* | tar -zxv
cd ..
fi



echo "2. build koreader.zip"
cd koreader
./kodev release kindle

