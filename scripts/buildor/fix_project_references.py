#!/usr/bin/python

import os;
import random;

class fix_project_references:
  """ This tool fixes visual studio 2010 projects to have the proper project references.

      Project files need to refer to other project files that they are dependent on if visual
      studio is to build them properly.  This is a painful task when done manually, but luckily
      this script automates the process for you.
      It requires an environment variable called BUILD_TOP that points at the top of all files
      included in a code base.  This is used to find the dependent projects.
  """
  
##############

  def __init__(self, argv):
    """ Initializes the class with a set of arguments to work with.

        The arguments need to be in the form described by print_instructions().
    """    
    self.arguments = argv  # remembers the command line for us.
    self.file_buffer = ""  # use for file i/o in the class.
    # initializes the list of projects found for the current source code hierarchy.
#hmmm: hier top denoted right now by the variable BUILD_TOP.  may want to document that.
    src_dir = os.getenv("BUILD_TOP")
    # projects and assets should keep in step, where project[x] is known to create asset[x].
    self.projects = self.walk_directory_for_projects(src_dir)  # list of project files.
    self.assets = self.locate_all_assets()  # dictionary of assets created by project files.

##############

#fix
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

  #fix
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

##############

#hmmm: are these good enough to become part of a file library?

  def read_file_data(self, filename):
    """ loads the file into our memory buffer for processing. """
    try:
      our_file = open(filename, "rb")
      try:
        self.file_buffer = our_file.read()
      except IOError:
        print("There was an error reading the file {0}".format(filename))
        return False
      finally:
        our_file.close()              
    except IOError:
      print("There was an error opening the file {0}".format(filename))
      return False
    self.file_lines = self.file_buffer.splitlines()
    return True

  def write_file_data(self, filename):
    """ takes the processed buffer and sends it back out to the filename. """
#    output_filename = filename + ".new"  # safe testing version.
    output_filename = filename
    try:
      our_file = open(output_filename, "wb")
      try:
        self.file_buffer = our_file.write(self.processed_buffer)
      except IOError:
        print("There was an error writing the file {0}".format(output_filename))
        return False
      finally:
        our_file.close()              
    except IOError:
      print("There was an error opening the file {0}".format(output_filename))
      return False
    return True

##############

  #unused?
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

  #unused?
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

  #unused?
  def process_file_data(self):
    """ iterates through the stored version of the file and replaces the phrase. """
    self.state = self.EATING_NORMAL_TEXT;
    # clear out any previously processed text.
    self.processed_buffer = ""   # reset our new version of the file contents.
    self.normal_accumulator = ""
    self.comment_accumulator = ""
    # iterate through the file's lines.
    contents = self.file_lines
    while (len(contents) > 0):
      # get the next line out of the input.
      next_line = contents[0]
      # drop that line from the remaining items.
      contents = contents[1:]
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

  #use for example.
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
  
##############

  def repair_project_references(self):
    """ the main entry point to the project fixing process.

    Operates on one project file at a time by:
      1) finding all libraries (lib files) used by the project A,
      2) locating the external project that creates each lib file,
      3) adding a reference to the external projects to our project A.

    We rely on some important assumptions to get this done:
      1) project names and project file names are unique across the codebase,
      2) the structure of the source code hierarchies uses a compatible architecture,
(which is?)

    """
    print("repair is unimplemented")

##############

  def extract_xml_tag(self, file_line, tag_name):
    """ locates an XML tag with "tag_name" and returns the contents of the tag.

    this currently assumes that the start tag, contents, and end tag are all on the same
    line of text (which is not a very good assumption in general).
    """
    if ('<' + tag_name in file_line):
      # find the greater than character.
      gt_pos = file_line.find('>', 0)
      if (gt_pos < 0): return ""  # fail.
      tag_close_pos = file_line.find('</' + tag_name, gt_pos + 1);
      if (tag_close_pos < 0): return ""  # fail.
      return file_line[gt_pos + 1 : tag_close_pos]
    return "" # failed to find anything relevant.

  def extract_xml_attribute(self, file_line, tag_name, attribute_name):
    """ locates an XML tag with "tag_name" and returns the value of the "attribute_name".

    """
    if ('<' + tag_name not in file_line): return ""
    if (attribute_name not in file_line): return ""
    attrib_pos = file_line.find(attribute_name, 0)
    # find the first quote around the attribute.
    quote_pos = file_line.find('"', attrib_pos)
    if (quote_pos < 0): return ""  # fail.
    quote_close_pos = file_line.find('"', quote_pos + 1)
    if (quote_close_pos < 0): return ""  # fail.
    return file_line[quote_pos + 1 : quote_close_pos]

##############

  def extract_xml_tag_from_file(self, filename, tag_name):
    """ reads in a file and extracts the contents of a particular XML tag.

may not want a file read here.  better to have a nice established way for
dealing with the existing buffer.
    """
    self.read_file_data(filename)
    contents = self.file_lines
    while (len(contents) > 0):
      # get the next line out of the input.
      next_line = contents[0]
      # drop that line from the remaining items.
      contents = contents[1:]
      #hmmm: maybe bad assumption thinking all on one line...
      found = self.extract_xml_tag(next_line, tag_name)
      if (found != ""): return found
    return "" # failed to find anything relevant.

##############

  def extract_guid_from_project_file(self, filename):
    """ reads in a visual studio project file and figures out that project's GUID.

    note that this will fail horribly if the project has been messed with and is no
    longer in microsoft's official format.
    """
    return self.extract_xml_tag_from_file(filename, 'ProjectGuid')

  def extract_filename_from_project_reference(self, file_line):
    """ given a ProjectReference line, this pulls out just the filename involved.
    """
    return self.extract_xml_attribute(file_line, "ProjectReference", "Include")

  def find_all_project_references(self, filename):
    """ reads in a visual studio project file and locates all references.
    """
    self.read_file_data(filename)
    contents = self.file_lines
    refs_list = []
    while (len(contents) > 0):
      # get the next line out of the input.
      next_line = contents[0]
      # drop that line from the remaining items.
      contents = contents[1:]
      ref = self.extract_filename_from_project_reference(next_line)
      if (ref != ''):
        #print("got reference: " + os.path.basename(ref) + " => " + ref)
        refs_list.append(ref)
    return refs_list

  def parse_dependency_line(self, line):
    """ given an AdditionalDependencies line, this finds the libs listed.
    """
    just_libs = self.extract_xml_tag(line, "AdditionalDependencies")
    if (just_libs == ""): return []
    lib_list = just_libs.split(';')
    # here we scan the list and remove any known-stupid entries.
    for i in range(1, len(lib_list)):
      if (lib_list[i] == '%(AdditionalDependencies)'):
        lib_list = lib_list[0 : i] + lib_list[i + 1 : len(lib_list)]
        i -= 1
    return lib_list

  #hmmm: could be a problem if the debug and release values differ.
  def extract_dependencies(self, filename):
    """ reads in a visual studio project file and locates all dependencies.

    This will produce a list of the lib files used by c++ projects.  These
    are what we need to link up to their providing projects, if they're
    actually things that we build.
    """
    self.read_file_data(filename)
    contents = self.file_lines
    while (len(contents) > 0):
      # get the next line out of the input.
      next_line = contents[0]
      # drop that line from the remaining items.
      contents = contents[1:]
      if ('AdditionalDependencies' in next_line):
        deps = self.parse_dependency_line(next_line)
        return deps
    return ()  # failed to find right line.

##############

#hmmm: could also be a problem if the debug and release values differ.
  def find_asset_created(self, filename):
    """ determines the asset created by a visual studio project file.

    This probably only works right on c++ projects.  It will figure out the
    item being created by the project using the breadcrumbs provided.
    """

    # these will need to be filled for us to have correctly determined what the project creates.
    project_name = ""
    config_type = ""

    self.read_file_data(filename)
    contents = self.file_lines
    while (len(contents) > 0):
      # get the next line out of the input.
      next_line = contents[0]
      # drop that line from the remaining items.
      contents = contents[1:]

      # we need the ProjectName or RootNamespace.
      #hmmm: csproj seems like they will work with this scheme already.
      if project_name == "":
        temp = self.extract_xml_tag(next_line, "RootNamespace")
        if (temp != ""):
          project_name = temp
#          print("found project name of " + project_name)
          continue

      # we look for the ConfigurationType, which tells us:
      #   DynamicLibrary | Library | StaticLibrary | Application | WinExe | Exe | Utility
      if config_type == "":
        temp = self.extract_xml_tag(next_line, "ConfigurationType")
        if (temp != ""):
          config_type = temp
#          print("found config type of " + config_type)
          continue
        temp = self.extract_xml_tag(next_line, "OutputType")
        if (temp != ""):
          config_type = temp
#          print("found output type of " + config_type)
          continue

      if (config_type != "") and (project_name != ""):
        asset_name = project_name
        if (config_type == "DynamicLibrary"): asset_name += ".dll"
        elif (config_type == "Library"): asset_name += ".dll"
        elif (config_type == "StaticLibrary"): asset_name += ".lib"
        elif (config_type == "Application"): asset_name += ".exe"
        elif (config_type == "WinExe"): asset_name += ".exe"
        elif (config_type == "Exe"): asset_name += ".exe"
        elif (config_type == "Utility"): return ""
        else:
          print("unknown configuration type: " + config_type + "\nin proj file: " + filename)
          return ""
        # we think we're successful in figuring out what should be created.
        return asset_name

    return ""  # failed to find right lines.

##############

  def walk_directory_for_projects(self, dir):
    """ traverses the directory in "dir" and finds all the project files.

    the project files are returned as a massive list.
    """

#hmmm: important optimization for walk; if the file where we store these things exists,
#      read directly out of that file instead of redoing hierarchy.  people can
#      delete the file when they want a fresh look.

    to_return = []

    for root, dirs, files in os.walk(dir):
#demo code from web:
#      print root, "consumes",
#      print sum(os.path.getsize(os.path.join(root, name)) for name in files),
#      print "bytes in", len(files), "non-directory files"
      if 'CVS' in dirs:
        dirs.remove('CVS')  # don't visit CVS directories.
      elif '.svn' in dirs:
        dirs.remove('.svn')  # don't visit subversion directories either.
      for curr_file in files:
        indy = len(curr_file) - 4
        # see if the file has the right ending to be a visual studio project file.
        if curr_file[indy:].lower() == 'proj':
          full_path = os.path.join(root, curr_file)
#          print("full path: " + full_path)
          # now add this good one to the returned list of projects.
          to_return.append(full_path)
    return to_return

  def find_relative_path(self, our_path, project_file):
    """ calculates path between directory at "our_path" to the location of "project_file".

    this assumes that the locations are actually rooted at the same place; otherwise there is
    no path between the locations.  the location at "our_path" is considered to be the source,
    or where we start out.  the location for "project_file" is the target location.
    """
    
    # canonicalize these to be linux paths.  we want to be able to split on forward slashes.
    sourcel = our_path.replace('\\', '/')
    targee = project_file.replace('\\', '/')
    # fracture the paths into their directory and filename components.
    sourcel = str.split(sourcel, '/')
    targee = str.split(targee, '/')
    # remove last item, which should be project filename.
    targee.pop()
    # destroy identical elements until items don't match or one path is gone.
    while (len(sourcel) and len(targee) and (sourcel[0] == targee[0])):
      sourcel.pop(0)
      targee.pop(0)
#    print("after dinner, sourcel now: " + " / ".join(sourcel))
#    print("and targee now: " + " / ".join(targee))
    # we comput the directory prefix of dot-dots based on the number of elements left
    # in the source path.
    prefix = ""
    for i in range(0, len(sourcel)):
      prefix += "../"
    print("calculated a prefix of: " + prefix)
#prove it is right in unit test.
    return prefix + "/".join(targee)

##############

  def locate_all_assets(self):
    """ locates every project file in our list and determines the asset created by it.

    this returns a dictionary of {asset=project} items.  we index by asset way more frequently
    than by project, so the asset name is used as our key.
    """
    to_return = {}
    for proj in self.projects:
      asset_found = self.find_asset_created(proj)
      # make sure we don't record a bogus project with no useful asset.
      if (asset_found == ""):
        self.projects.remove(proj)  # should be safe for our list iteration.
        continue  # skip adding the bogus one.
      to_return[asset_found] = proj
#      print("proj " + proj + " creates: " + asset_found)
    return to_return

##############

  def locate_referenced_projects(self, project):
    """ finds all the libraries needed by the "project" file and returns their project files.
    """
    to_return = []
    # find the libraries and such used by this project.
    libs = self.extract_dependencies(project)
#    print("was told the libs used by " + project + " are:\n" + " ".join(libs))
    # now find who creates those things.
    for current in libs:
      # if we know of the library in our list of assets, then we can use it right away.
      if current in self.assets:
        # this item exists and is created by one of our projects.
        proj_needed = self.assets[current]
#        print("asset '" + current + "' created by: " + proj_needed)
        to_return.append(proj_needed)
        continue
      # if we didn't find the thing with it's current name, we'll see if we can convert
      # it into a dll and look for that instead.
#      print("looking at piece: " + current[len(current) - 4:])
      if current[len(current) - 4:] == ".lib":
#        print("found rogue we can convert to a dll to find: " + current)
        current = current[0:-4] + ".dll"
#        print("new name: " + current)
        if current in self.assets:
          proj_needed = self.assets[current]
#          print("found asset '" + current + "' created by: " + proj_needed)
          to_return.append(proj_needed)
          continue
#        else:
#          print("could not find '" + current + "' as an asset that we know how to create.");
    return to_return
    
  def remove_redundant_references(self, project):
    """ cleans out any references in "project" to assets that we intend to update.

    this actually modifies the file.  it had better be right.
    """
#load file data for the thing
#find references
  #see if reference is one we know about
  #if so, zap it out of file contents
#write file back out

##############

  def unit_test(self):
    """ a sort-of unit test for the functions in this script.

    currently geared for manual inspection of the test results.
    """
    print("testing some of the methods...")
    test_file = ""
    if len(self.arguments) > 1:
      test_file = self.arguments[1]
    if test_file == "": test_file = os.getenv("REPOSITORY_DIR") + "/source/core/applications/nechung/nechung.vcxproj"
    print("test file is: " + test_file)

    guid = self.extract_guid_from_project_file(test_file)
    print("from proj, got a guid of " + guid)

    refs = self.find_all_project_references(test_file)
    print("refs list is: " + " ".join(refs))

#    libs = self.extract_dependencies(test_file)
#    print("was told the libs used are: " + " ".join(libs))

    asset = self.find_asset_created(test_file)
    print("our created asset is: " + asset)

#    print("walked directories got:\n" + " ".join(fixit.projects))

#    print("assets found are:\n" + " ".join(fixit.assets))

    if (len(fixit.projects) > 0):
      rando = random.randint(0, len(fixit.projects) - 1)
      print("index chosen to examine: {0}".format(rando))

      relpath = self.find_relative_path(os.path.dirname(test_file), fixit.projects[rando])
      print("found relative path from source:\n  " + test_file)
      print("to target:\n  " + fixit.projects[rando])
      print("is this =>\n  " + relpath)

    full_refs = self.locate_referenced_projects(test_file)
    print("refs found are:\n" + " ".join(full_refs))

    self.remove_redundant_references(test_file)
    print("we just munged your file!  go check it!  no references should remain that are in our new list.")

#still needed:
# remove any existing references that we now have a replacement for.
#   base this on the basename of the project file?  blah.vcxproj already there e.g.
# spit out xml form of references for the dependent projects.
# put new references into the right place in file.


##############

# run the script if we are non-interactive.
if __name__ == "__main__":
    import sys
    fixit = fix_project_references(sys.argv)

    # comment this out when script is working.
    fixit.unit_test()

    print("we're bailing before doing anything real...")
    exit(3)

    fixit.repair_project_references()

##############

# parking lot of things to do in future:


