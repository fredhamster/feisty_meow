package org.gffs.version;

import java.io.File;
import java.io.IOException;

import org.gffs.application.ProgramTools;

public class VersionManager
{
	static private final String VERSION_FILENAME = "current.version";

	private Version _currentVersion;
	private File _versionFile;

	public VersionManager()
	{
		_versionFile = new File(ProgramTools.getInstallationDirectory(), VERSION_FILENAME);
		_currentVersion = null;
	}

	public Version getCurrentVersion() throws IOException
	{
		if (_currentVersion == null) {
			// we go with the installer scheme to start with, where there's a current.version in the
			// top-level.
			if (!_versionFile.exists()) {
				// try failing over to the source code's version of the file inside the installer
				// directory.
				_versionFile = new File(ProgramTools.getInstallationDirectory(), "installer/" + VERSION_FILENAME);
				if (!_versionFile.exists()) {
					_currentVersion = Version.EMPTY_VERSION;
				}
			}
			if (_versionFile.exists()) {
				_currentVersion = new Version(_versionFile);
			}
		}

		return _currentVersion;
	}
}
