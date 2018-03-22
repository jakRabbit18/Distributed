# this makes all the test files used in development

for x in {1..50}
do 
	echo "This is just a test"$x >> mytest$x.txt
done

for x in {1..3}
do
	mkdir testdir$x
	cd testdir$x
		for y in {1..20}
		do
			echo $y
			echo "This is second level test" >> mytest$y.txt
		done

		for y in {1..3}
		do
			echo $y
			mkdir testdir$y
			cd testdir$y

				for z in {1..10}
				do
					echo "This a third level test" >> mytest$z.txt
				done

			cd ..
		done
	cd ..
done

