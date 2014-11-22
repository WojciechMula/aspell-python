# This is simple script that tests aspell-python module.

# import module
import aspell

#speller = aspell.Speller()

for item in aspell.ConfigKeys():
	print item
