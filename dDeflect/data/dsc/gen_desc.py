import sys
import os
import json

# responsible for removing description files from specified folder recursively
def clean(path, suff_dsc, desc_fld):
	for root, dirs, files in os.walk(path):
		for file in files:
			full_path = root + "/" + file 
			if file.endswith(suff_dsc) and os.path.isfile(full_path):
				os.remove(full_path)

# responsible for generating .dsc description files
def generate_desc(path, suff_re, suff_dsc, desc_fld):
	json_desc = dict(method_name = "dummy", description = "dummy", debugged = "dummy", path = "dummy")
	for root, dirs, files in os.walk(path):
		for file in files:
			if file.endswith(suff_re):
				# TODO: fill other keys
				json_desc["path"] = root + "/" + file 
				#print file
				full_desc_path = root + "/" + desc_fld
				#print full_desc_path
				if not os.path.exists(full_desc_path):
					os.mkdir(full_desc_path, 0755)
				if os.path.isdir(full_desc_path):
					print file
					with open(full_desc_path + "/" + file[:file.find(suff_re)] + suff_dsc, 'w') as desc_file:
						json.dump(json_desc, desc_file)

# gets config from file and executes specified command
def get_config(path, command):
	json_config = open(path, 'r')
	jconfig = json.load(json_config)
	json_config.close()
	if not 'anti_re_path' in jconfig:
		print 'anti_re_path := path to anti-re source files should be specified'
		sys.exit()
	if not 'anti_re_suf_src' in jconfig and command != create:
		print 'anti_re_suf_src := anti-re suffix source files should be specified'
		sys.exit()
	if not 'anti_re_suf_dsc' in jconfig:
		print 'anti_re_suf_dsc := anti-re suffix description files should be specified'
		sys.exit()
	if not 'dsc_fld' in jconfig:
		print 'dsc_fld := relative path (source file) to describe folder should be specified'
		sys.exit()

	if command == "create":
		generate_desc(jconfig['anti_re_path'], jconfig['anti_re_suf_src'], jconfig['anti_re_suf_dsc'], jconfig['dsc_fld'])
	elif command == "clean":
		clean(jconfig['anti_re_path'], jconfig['anti_re_suf_dsc'], jconfig['dsc_fld'])

# without args
if len(sys.argv) < 2:
	print "Usage: gen_desc.py command := 'clean | create(by def)' <json_config>"
	sys.exit()

# second one was not create
if sys.argv == 3 and (sys.argv[1] != "create" or sys.argv[1] != "clean"):
	print "Usage: gen_desc.py <command := 'clean | create(by def)'> <json_config>"
	sys.exit()

get_config(sys.argv[2], sys.argv[1])
