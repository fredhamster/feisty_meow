#!/usr/bin/python

class phrase_replacer:
  """ A simple replacement tool that honors some C/C++ syntax when replacing.

      This will take a particular phrase given by the user and find it in a set of
      documents.  That phrase will be replaced when it appears completely, and is not
      in a C or C++ style comment (// or /* ... */).  It also must be clear of any
      other alphanumeric pollution, and only be surrounded by white space or operation
      characters.
  """
  
  def __init__(self, argv):
    """ Initializes the class with a set of arguments to work with.

        The arguments need to be in the form described by print_instructions().
    """    
    self.arguments = argv
    # we have three states for the processing: consuming normal code (not within a comment),
    # consuming a single line comment, and consuming a multi-line comment.
    self.EATING_NORMAL_TEXT = 0
    self.EATING_ONELINE_COMMENT = 1
    self.EATING_MULTILINE_COMMENT = 2

  def print_instructions(self):
    """ Shows the instructions for using this class. """
    print("""
This script will replace all occurrences of a phrase you specify in a set of files.  The
replacement process will be careful about C and C++ syntax and will not replace occurrences
within comments or which are not "complete" phrases (due to other alpha-numeric characters
that abut the phrase).  The arguments to the script are:

  {0}: PhraseToReplace  ReplacementPhrase  File1  [File2 ...]

For example, if the phrase to replace is Goop, it will be replaced in these contexts:
  Goop[32]
  molo-Goop
  *Goop
but it will not be found in these contexts:
  // doop de Goop
  rGoop
  Goop23
""".format(self.arguments[0]))

  def validate_and_consume_command_line(self):
    """ Performs command line argument handling. """
    arg_count = len(self.arguments)
#    for i in range(1, arg_count):
#      print("i is {0}, arg is {1}".format(i, self.arguments[i]))
    # we need more than 2 arguments, since there needs to be at least one file also.
    if arg_count < 4:
      return False
    self.phrase_to_replace = self.arguments[1]
    self.replacement_bit = self.arguments[2]
    print("got phrase to replace: \'{0}\' and replacement: \'{1}\'".format(self.phrase_to_replace, self.replacement_bit))
    self.files = self.arguments[3:]
    return True

  def read_file_data(self, filename):
    """ loads the file into our memory buffer for processing. """
    try:
      our_file = open(filename, "rb")
      try:
        file_buffer = our_file.read()
      except IOError:
        print("There was an error reading the file {0}".format(filename))
        return False
      finally:
        our_file.close()              
    except IOError:
      print("There was an error opening the file {0}".format(filename))
      return False
    self.file_lines = file_buffer.splitlines()
    return True

  def write_file_data(self, filename):
    """ takes the processed buffer and sends it back out to the filename. """
#    output_filename = filename + ".new"  # safe testing version.
    output_filename = filename
    try:
      our_file = open(output_filename, "wb")
      try:
        file_buffer = our_file.write(self.processed_buffer)
      except IOError:
        print("There was an error writing the file {0}".format(output_filename))
        return False
      finally:
        our_file.close()              
    except IOError:
      print("There was an error opening the file {0}".format(output_filename))
      return False
    return True

  def is_alphanumeric(self, check_char):
    """ given a character, this returns true if it's between a-z, A-Z or 0-9. """
    if (check_char[0] == "_"):
      return True
    if ( (check_char[0] <= "z") and (check_char[0] >= "a")):
      return True
    if ( (check_char[0] <= "Z") and (check_char[0] >= "A")):
      return True
    if ( (check_char[0] <= "9") and (check_char[0] >= "0")):
      return True
    return False

  def replace_within_string(self, fix_string):
    """ given a string to fix, this replaces all appropriate locations of the phrase. """
    indy = 0
#    print("got to replace within string")
    while (indy < len(fix_string)):
      # locate next occurrence of replacement text, if any.
      indy = fix_string.find(self.phrase_to_replace, indy)
#      print("find indy={0}".format(indy))
      if (indy > -1):
#        print("found occurrence of replacement string")
        # we found an occurrence, but we have to validate it's separated enough.
        char_before = "?"  # simple default that won't fail our check.
        char_after = "?"
        if (indy > 0):
          char_before = fix_string[indy-1]
        if (indy + len(self.phrase_to_replace) < len(fix_string) - 1):
          char_after = fix_string[indy+len(self.phrase_to_replace)]
#        print("char before {0}, char after {1}".format(char_before, char_after))
        if (not self.is_alphanumeric(char_before) and not self.is_alphanumeric(char_after)):
          # this looks like a good candidate for replacement.
          fix_string = "{0}{1}{2}".format(fix_string[0:indy], self.replacement_bit, fix_string[indy+len(self.phrase_to_replace):])
#          print("changed string to: {0}".format(fix_string))
      else:
        break
      indy += 1  # no matches means we have to keep skipping forward.
    return fix_string  # give back processed form.

  def emit_normal_accumulator(self):
    """ handle emission of a chunk of normal code (without comments). """
    # process the text to perform the replacement...
    self.normal_accumulator = self.replace_within_string(self.normal_accumulator)
    # then send the text into our main buffer; we're done looking at it.
    self.processed_buffer += self.normal_accumulator
    self.normal_accumulator = ""

  def emit_comment_accumulator(self):
    """ emits the piled up text for comments found in the code. """
    self.processed_buffer += self.comment_accumulator
    self.comment_accumulator = ""

  def process_file_data(self):
    """ iterates through the stored version of the file and replaces the phrase. """
    self.state = self.EATING_NORMAL_TEXT;
    # clear out any previously processed text.
    self.processed_buffer = ""   # reset our new version of the file contents.
    self.normal_accumulator = ""
    self.comment_accumulator = ""
    # iterate through the file's lines.
    while (len(self.file_lines) > 0):
      # get the next line out of the input.
      next_line = self.file_lines[0]
      # drop that line from the remaining items.
      self.file_lines = self.file_lines[1:]
#      print("next line: {0}".format(next_line))
      # decide if we need a state transition.
      indy = 0
      if ((len(next_line) > 0) and (self.state == self.EATING_NORMAL_TEXT) and ('/' in next_line)):
        # loop to catch cases where multiple slashes are in line and one IS a comment.
        while (indy < len(next_line)):
          # locate next slash, if any.
          indy = next_line.find('/', indy)
          if (indy < 0):
            break
          if ((len(next_line) > indy + 1) and (next_line[indy + 1] == '/')):
            # switch states and handle any pent-up text.
            self.normal_accumulator += next_line[0:indy]  # get last tidbit before comment start.
            next_line = next_line[indy:]  # keep only the stuff starting at slash.
            self.state = self.EATING_ONELINE_COMMENT
#            print("state => oneline comment")
            self.emit_normal_accumulator()
            break
          if ((len(next_line) > indy + 1) and (next_line[indy + 1] == '*')):
            # switch states and deal with accumulated text.
            self.normal_accumulator += next_line[0:indy]  # get last tidbit before comment start.
            next_line = next_line[indy:]  # keep only the stuff starting at slash.
            self.state = self.EATING_MULTILINE_COMMENT
#            print("state => multiline comment")
            self.emit_normal_accumulator()
            break
          indy += 1  # no matches means we have to keep skipping forward.

      # now handle things appropriately for our current state.
      if (self.state == self.EATING_NORMAL_TEXT):
        # add the text to the normal accumulator.
#        print("would handle normal text")
        self.normal_accumulator += next_line + "\n"
      elif (self.state == self.EATING_ONELINE_COMMENT):
        # save the text in comment accumulator.
#        print("would handle oneline comment")
        self.comment_accumulator += next_line + "\n"
        self.emit_comment_accumulator()
        self.state = self.EATING_NORMAL_TEXT
      elif (self.state == self.EATING_MULTILINE_COMMENT):
        # save the text in comment accumulator.
#        print("would handle multiline comment")
        self.comment_accumulator += next_line + "\n"
        # check for whether the multi-line comment is completed on this line.
        if ("*/" in next_line):
#          print("found completion for multiline comment on line.")
          self.emit_comment_accumulator()
          self.state = self.EATING_NORMAL_TEXT
    # verify we're not in the wrong state still.
    if (self.state == self.EATING_MULTILINE_COMMENT):
      print("file seems to have unclosed multi-line comment.")
    # last step is to spit out whatever was trailing in the accumulator.
    self.emit_normal_accumulator()
    # if we got to here, we seem to have happily consumed the file.
    return True

  def replace_all_occurrences(self):
    """ Orchestrates the process of replacing the phrases. """
    # process our command line arguments to see what we need to do.
    try_command_line = self.validate_and_consume_command_line()
    if (try_command_line != True):
      print("failed to process the command line...\n")
      self.print_instructions()
      exit(1)
    # iterate through the list of files we were given and process them.
    for i in range(0, len(self.files)):
      print("file {0} is \'{1}\'".format(i, self.files[i]))
      worked = self.read_file_data(self.files[i])
      if (worked is False):
        print("skipping since file read failed on: {0}".format(self.files[i]))
        continue
#      print("{0} got file contents:\n{1}".format(self.files[i], self.file_lines))
      worked = self.process_file_data()
      if (worked is False):
        print("skipping, since processing failed on: {0}".format(self.files[i]))
        continue
      worked = self.write_file_data(self.files[i])
      if (worked is False):
        print("writing file back failed on: {0}".format(self.files[i]))
    print("finished processing all files.")
  

if __name__ == "__main__":
    import sys
    slicer = phrase_replacer(sys.argv)
    slicer.replace_all_occurrences()

##############

# parking lot of things to do in future:

#hmmm: actually sometimes one DOES want to replace within comments.  argh.
#      make ignoring inside comments an optional thing.  later.

# hmmm: one little issue here is if the text to be replaced happens to reside on
#       the same line after a multi-line comment.  we are okay with ignoring that
#       possibility for now since it seems brain-dead to write code that way.


