package org.gffs.compression;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import org.apache.commons.compress.archivers.ArchiveEntry;
import org.apache.commons.compress.archivers.ArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.archivers.zip.ZipArchiveInputStream;
import org.apache.commons.compress.compressors.gzip.GzipCompressorInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class UnpackTar
{
	static private Log _logger = LogFactory.getLog(UnpackTar.class);

	/**
	 * takes a tar.gz file as the "tarFile" parameter, then decompresses and unpacks the file into the "dest" location.
	 */

	public enum archiveType {
		TAR,
		ZIP,
		TGZ
	};

	public static void uncompressArchive(ArchiveInputStream tarIn, File dest, archiveType archType, boolean grantUserPermsToGroup)
		throws IOException
	{

		if (dest.exists()) {
			// Don't unpack into an existing directory
			throw new IOException("Directory " + dest.getAbsolutePath() + " already exists. Unpacking exiting");
		}
		dest.mkdir();

		ArchiveEntry tarEntry = tarIn.getNextEntry();
		while (tarEntry != null) {
			// New code by ASG 2016-02-21. Added extracting user permission bits and OR ing them with group permissions
			int mode = 0;
			int defaultMode = 0750; // assume somewhat standard executable permissions if we cannot get the mode.
			switch (archType) {
				case TAR:
				case TGZ:
					mode = ((TarArchiveEntry) tarEntry).getMode();
					break;
				case ZIP:
					mode = ((ZipArchiveEntry) tarEntry).getUnixMode();
					break;
			}
			if (mode == 0) {
				mode = defaultMode;
			}
			if (_logger.isTraceEnabled())
				_logger.debug("The mode on '" + tarEntry.getName() + "' is " + Integer.toOctalString(mode));
			if (grantUserPermsToGroup) {
				int temp = mode & 0700;
				temp = temp / 8; // Shift it right 3 bit positions
				mode = mode | temp;
				if (_logger.isTraceEnabled())
					_logger.debug("Now mode on '" + tarEntry.getName() + "' is " + Integer.toOctalString(mode));
			}
			// End of extracting and Or ing the permission bits.

			// create a file with the same name as the tarEntry
			File destPath = new File(dest, tarEntry.getName());
			if (_logger.isTraceEnabled())
				_logger.debug("working on: " + destPath.getCanonicalPath());
			if (tarEntry.isDirectory()) {
				destPath.mkdirs();
			} else {
				destPath.createNewFile();

				// byte [] btoRead = new byte[(int)tarEntry.getSize()];
				byte[] btoRead = new byte[8192];
				BufferedOutputStream bout = new BufferedOutputStream(new FileOutputStream(destPath));
				int len = 0;
				boolean wroteAnything = false;
				while ((len = tarIn.read(btoRead)) != -1) {
					if (_logger.isTraceEnabled())
						_logger.debug("read " + len + " bytes");
					wroteAnything = true;
					bout.write(btoRead, 0, len);
				}
				if (!wroteAnything) {
					_logger.error("zero bytes read from: " + destPath.getCanonicalPath());
				}

				bout.close();
			}
			// using PosixFilePermission to set file permissions that we extracted earlier.
			HashSet<PosixFilePermission> perms = new HashSet<PosixFilePermission>();
			// add owners permission
			if ((mode & 0400) != 0)
				perms.add(PosixFilePermission.OWNER_READ);
			if ((mode & 0200) != 0)
				perms.add(PosixFilePermission.OWNER_WRITE);
			if ((mode & 0100) != 0)
				perms.add(PosixFilePermission.OWNER_EXECUTE);
			// add group permissions
			if ((mode & 0040) != 0)
				perms.add(PosixFilePermission.GROUP_READ);
			if ((mode & 0020) != 0)
				perms.add(PosixFilePermission.GROUP_WRITE);
			if ((mode & 0010) != 0)
				perms.add(PosixFilePermission.GROUP_EXECUTE);
			// add others permissions
			if ((mode & 0004) != 0)
				perms.add(PosixFilePermission.OTHERS_READ);
			if ((mode & 0002) != 0)
				perms.add(PosixFilePermission.OTHERS_WRITE);
			if ((mode & 0001) != 0)
				perms.add(PosixFilePermission.OTHERS_EXECUTE);

			Files.setPosixFilePermissions(Paths.get(destPath.getCanonicalPath()), perms);
			tarEntry = tarIn.getNextEntry();
		}
		tarIn.close();
	}

	public synchronized static void uncompressTarGZ(File tarFile, File dest, boolean grantUserPermsToGroup) throws IOException
	{
		TarArchiveInputStream tarIn = new TarArchiveInputStream(new GzipCompressorInputStream(new FileInputStream(tarFile)));

		uncompressArchive(tarIn, dest, archiveType.TGZ, grantUserPermsToGroup);
	}

	public synchronized static void uncompressTar(File tarFile, File dest, boolean grantUserPermsToGroup) throws IOException
	{
		TarArchiveInputStream tarIn = new TarArchiveInputStream(new FileInputStream(tarFile));

		uncompressArchive(tarIn, dest, archiveType.TAR, grantUserPermsToGroup);
	}

	public synchronized static void uncompressZip(File zipFile, File dest, boolean grantUserPermsToGroup) throws IOException
	{
		ZipArchiveInputStream tarIn = new ZipArchiveInputStream(new FileInputStream(zipFile));

		uncompressArchive(tarIn, dest, archiveType.ZIP, grantUserPermsToGroup);
	}

	static public void main(String[] args) throws Throwable
	{
		if (args.length != 2) {
			System.err.println("USAGE: UnpackTar {tar.gz file} {output location}");
			System.exit(1);
		}

		try {
			UnpackTar.uncompressTarGZ(new File(args[0]), new File(args[1]), false);
		} catch (Throwable t) {
			_logger.error("failed to uncompress tar file " + args[0] + " into " + args[1]);
			System.exit(1);
		}

		System.out.println("successfully uncompressed tar file " + args[0] + " into " + args[1]);
	}

}
