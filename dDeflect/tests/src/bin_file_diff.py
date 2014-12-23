import sys

def bin_file_diff(fn1, fn2):
	cont1 = ""
	cont2 = ""

	try:
		with open(fn1, "rb") as f1:
			cont1 = f1.read()
	except (OSError, IOError) as e:
		print "error while handling", fn1, ":", e.strerror
		sys.exit(127)

	try:
		with open(fn2, "rb") as f2:
			cont2 = f2.read()
	except (OSError, IOError) as e:
		print "error while handling ", fn2, ": ", e.strerror
		sys.exit(127)

	# compare files in binary
	len1 = len(cont1)
	len2 = len(cont2)
	min_len = len1 if len1 < len2 else len2
	# print min_len

	for i in range(min_len):
		#print type(ord(cont1[i]))
		if ord(cont1[i]) != ord(cont2[i]):
			print "offset", hex(i), ":", hex(ord(cont1[i])), "!=", hex(ord(cont2[i]))

def main():
	if len(sys.argv) != 3:
		print "usage: bin_file_diff file1 file2"
		sys.exit(127)
	bin_file_diff(sys.argv[1], sys.argv[2])
	
if __name__ == "__main__":
	main()


