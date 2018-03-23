for x in {1..100}
do
mkdir jack
cd jack
mkdir barry
echo "this is the second version" >> test.txt
cd barry
echo "this is the second version" >> test.txt
echo "this is the first version" >> test1.txt
cd ..
cd ..
rm jack
DUMP
done

