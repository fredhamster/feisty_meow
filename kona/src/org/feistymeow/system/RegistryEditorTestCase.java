package org.feistymeow.system;

import static org.junit.Assert.*;
import org.junit.*;

import org.feistymeow.system.RegistryEditor;

/**
 * RegistryEditor junit tests.
 * 
 * @author Chris Koeritz
 *
 */
public class RegistryEditorTestCase 
{    
	@Before
	public void setUp() throws Exception 
	{
	}

	@After
	public void tearDown() throws Exception 
	{
	}

	@Test
	public void testCheckKey_missing() {
		// this key cannot exist, since we're not allowed to write at that
		// top level hive (as far as we know).
		assertFalse(RegistryEditor.checkKey("HKLM\\flaubert\\maximus"));
	}

	@Test
	public void testCheckKey_present() {
		// this key must always exist as far as we know.
		assertTrue(RegistryEditor.checkKey("HKLM\\Software"));
	}

	@Test
	public void testCheckKey_bad() {
		assertFalse(RegistryEditor.checkKey(null));
	}

	@Test
	public void testGetValue_present() {
		String PERSONAL_FOLDER_KEY = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
		String PERSONAL_FOLDER_VALUE = "Personal";
		assertNotNull(RegistryEditor.getValue(PERSONAL_FOLDER_KEY, PERSONAL_FOLDER_VALUE,
				RegistryEditor.STRING_TYPE)); 
	}

	@Test
	public void testGetValue_missing() {
		String PERSONAL_FOLDER_KEY = "HKCU\\Flombix\\Gruntnork";
		String PERSONAL_FOLDER_VALUE = "Personalish";
		assertNull(RegistryEditor.getValue(PERSONAL_FOLDER_KEY, PERSONAL_FOLDER_VALUE,
				RegistryEditor.STRING_TYPE)); 
	}

	@Test
	public void testSetValueAndRemove_AllWork()
	{
		String EXAMPLE_KEY_ROOT = "HKCU\\Software\\SpunkBaster5000";
		String EXAMPLE_KEY = EXAMPLE_KEY_ROOT + "\\traumix";
		String EXAMPLE_VALUE = "glonkish";
		String EXAMPLE_CONTENTS1 = "ralphWiggum!";
		String EXAMPLE_CONTENTS2 = "moeSzyslak?";

		// first try deleting the value.  this should fail to start with.
		assertFalse(RegistryEditor.deleteValue(EXAMPLE_KEY, EXAMPLE_VALUE));
		// remove the entire key if present.
		assertFalse(RegistryEditor.deleteKey(EXAMPLE_KEY_ROOT));
		// now test that it really doesn't seem to be there.
		assertFalse(RegistryEditor.checkKey(EXAMPLE_KEY));
//System.out.println("now setting key [" + EXAMPLE_KEY + "] value [" + EXAMPLE_VALUE + "] to '" + EXAMPLE_CONTENTS1 + "'");
		// now try adding the example value.
		assertTrue(RegistryEditor.setValue(EXAMPLE_KEY, EXAMPLE_VALUE, RegistryEditor.STRING_TYPE, EXAMPLE_CONTENTS1));
		// check that the contents are what we expect.
		assertEquals(RegistryEditor.getValue(EXAMPLE_KEY, EXAMPLE_VALUE, RegistryEditor.STRING_TYPE),
				EXAMPLE_CONTENTS1);
		// now change the value's contents to a new setting.
		assertTrue(RegistryEditor.setValue(EXAMPLE_KEY, EXAMPLE_VALUE, RegistryEditor.STRING_TYPE, EXAMPLE_CONTENTS2));
		// make sure the update has succeeded.
		assertEquals(RegistryEditor.getValue(EXAMPLE_KEY, EXAMPLE_VALUE, RegistryEditor.STRING_TYPE),
				EXAMPLE_CONTENTS2);
		// now whack the value we had added.
		assertTrue(RegistryEditor.deleteValue(EXAMPLE_KEY, EXAMPLE_VALUE));
		// now remove the entire key we added.
		assertTrue(RegistryEditor.deleteKey(EXAMPLE_KEY_ROOT));
	}

}

