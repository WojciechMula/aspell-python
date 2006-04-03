# This is simple script that tests aspell-python module.

# import module
import aspell

# select english dictionary
speller = aspell.Speller('lang', 'en') 

# get current configuration of the speller, and create dictionary
speller_config = dict( [(name, value) for name, type, value in speller.ConfigKeys()])

##################################################
print "="*60
print "check method:"

words = [
	'word',
	'flower',
	'misteke', # mistake
	'mistake',
	'tree',
	'ruck',
	'rock',
	'rack',
	'zo' # zoo
]

def pretty_print(wordlist):
	for word in wordlist:
		print "* '%s':" % word,
		if speller.check(word):
			print "ok (1)"
		else:
			print "wrong (0)"

pretty_print(words)

##################################################
print "="*60
print "suggest method:"

words = [
	'wrod',
	'tre',
	'zo',
]

for word in words:
	print "* '%s': %s" % (word, ', '.join(speller.suggest(word)))

##################################################
print "="*60
print "addReplacement method:"

wrong_word   = 'tre'
correct_word = 'tree'
print "default order of word list returned by suggest:"
print "* '%s': %s" % (wrong_word, ', '.join(speller.suggest(wrong_word))) 
print "order after calling addReplacement('%s', '%s'):" % (wrong_word, correct_word) 

speller.addReplacement(wrong_word, correct_word)
print "* '%s': %s" % (wrong_word, ', '.join(speller.suggest(wrong_word))) 

##################################################
print "="*60
print "addtoSession method:"

unknown_but_correct_words = ['linux', 'aspell', 'emacs', 'ocaml', 'python']

print "aspell dosn't know any of these words:"
pretty_print(unknown_but_correct_words)

print "now, we add them to session dictionary"
for word in unknown_but_correct_words:
	speller.addtoSession(word)
print "and check once again"
pretty_print(unknown_but_correct_words)

##################################################
print "="*60
print "getSessionwordlist method:"

print "let see the words we've added earlier:", speller.getSessionwordlist()

##################################################
print "="*60
print "clearSession method:"

print "now, we clean session dictionary:"
speller.clearSession()
pretty_print(unknown_but_correct_words)

##################################################
print "="*60
print "addtoPersonal method:"

print "aspell was forgot words, but now we use personal dictionary"
for word in unknown_but_correct_words:
	speller.addtoPersonal(word)

pretty_print(unknown_but_correct_words)

##################################################
print "="*60
print "saveAllwords method:"

def cat(filename):
	try:
		file = open(filename)
	except:
		print "Can't open file '%s'" % filename
	else:
		print "--- %s ---" % filename
		for line in open(filename):
			print line,	
		print "--- eof ---"
		file.close()

print "aspell saves personal dictionary in: '%s'" % \
	speller_config['personal-path'],

print "let see what words are stored in the file:"
cat(speller_config['personal-path'])

print "now we save personal dictionary, and see again contents of the file"
speller.saveAllwords()
cat(speller_config['personal-path'])

##################################################
print "="*60
print "getPersonalwordlist method:"

speller.addtoPersonal('TheEnd')
print "method returns:", speller.getPersonalwordlist()
