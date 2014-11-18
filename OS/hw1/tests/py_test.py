f = open("test_out.txt", "r")

c = 0

for i in f.read():
	if i != chr(0):
		print ("error\n: " + chr(i) + "\n")
		break

print("Done Successfully!!!\nNo Errors found in test\n")

f.close()
