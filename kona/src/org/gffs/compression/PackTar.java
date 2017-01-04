package org.gffs.compression;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;

import org.apache.commons.compress.archivers.ArchiveEntry;
import org.apache.commons.compress.archivers.ArchiveOutputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;
import org.apache.commons.compress.compressors.gzip.GzipCompressorOutputStream;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.IOFileFilter;
import org.apache.commons.io.filefilter.TrueFileFilter;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class PackTar
{
	static private Log _logger = LogFactory.getLog(PackTar.class);

	/**
	 * returns the "longName" with the "prefix" stripped off the front. if the name doesn't contain the prefix, then an error is thrown.
	 */
	public static String stripOutPrefix(String prefix, String longName) throws IOException
	{
		int indy = longName.indexOf(prefix, 0);
		if (indy < 0)
			throw new IOException("failure to find prefix in string, prefix=" + prefix + " and string=" + longName);
		return longName.substring(indy + prefix.length());
	}

	/**
	 * adds the file pointed at by "source" to the archive in "tarOut" as a compressed file. if "source" is a directory, then the directory is
	 * recursed and added. the names in the archive will be stripped of the "prefix" string before being added to the archive; for example, if
	 * the source path "/home/fred/wumpus" and the prefix string is "/home/fred/", then the files in the archive will be named "wumpus/A",
	 * "wumpus/B", etc.
	 */
	public static void compressArchive(ArchiveOutputStream tarOut, String prefix, File source) throws IOException
	{
		_logger.debug("entered into compress archive on source " + source + " prefix " + prefix + " and tarout " + tarOut);

		if (!source.exists()) {
			// Don't unpack into an existing directory.
			String msg = "Directory " + source.getAbsolutePath() + " doesn't exist yet. Cannot pack.";
			_logger.error(msg);
			throw new IOException(msg);
		}

		// traverse the whole tree of the directory (or just write this directly if it's a file).
		if (source.isFile()) {
			String choppedName = stripOutPrefix(prefix, source.getPath());
			if (_logger.isDebugEnabled())
				_logger.debug("adding a file to the archive (chopped): " + choppedName);
			ArchiveEntry f = tarOut.createArchiveEntry(source, choppedName);
			tarOut.putArchiveEntry(f);
			IOUtils.copy(new FileInputStream(source), tarOut);
			tarOut.closeArchiveEntry();
		} else if (source.isDirectory()) {
			// traverse the directory tree at just this height and add everything recursively.

			if (_logger.isDebugEnabled())
				_logger.debug("iterating over a directory to add its contents to the archive: " + source);

			Iterator<File> spidey = FileUtils.iterateFiles(source, new IOFileFilter()
			{
				@Override
				public boolean accept(File arg0)
				{
					return true;
				}

				@Override
				public boolean accept(File arg0, String arg1)
				{
					return true;
				}
			}, TrueFileFilter.INSTANCE);

			File item = null;
			while (spidey.hasNext()) {
				item = spidey.next();

				if (_logger.isTraceEnabled())
					_logger.debug("recursing on item: " + item);

				compressArchive(tarOut, prefix, item);
			}
		} else {
			String msg = "source is not a file or directory although it exists.  unknown how to process.";
			_logger.error(msg);
			throw new IOException(msg);
		}

	}

	/**
	 * returns the same string as presented, but ensures that the last character will be a directory separator (/).
	 */
	public static String findAppropriatePrefix(String toChop)
	{
		if (toChop.endsWith("/"))
			return toChop; // already ready.
		else
			return toChop + "/"; // add a slash on the end.
	}

	public synchronized static void compressTarGZ(File tarFile, File dest) throws IOException
	{
		TarArchiveOutputStream tarOut = new TarArchiveOutputStream(new GzipCompressorOutputStream(new FileOutputStream(tarFile)));
		compressArchive(tarOut, findAppropriatePrefix(dest.getPath()), dest);
		tarOut.close();
	}

	public synchronized static void compressTar(File tarFile, File dest) throws IOException
	{
		TarArchiveOutputStream tarOut = new TarArchiveOutputStream(new FileOutputStream(tarFile));
		compressArchive(tarOut, findAppropriatePrefix(dest.getPath()), dest);
		tarOut.close();
	}

	public synchronized static void compressZip(File zipFile, File dest) throws IOException
	{
		ZipArchiveOutputStream tarOut = new ZipArchiveOutputStream(new FileOutputStream(zipFile));
		compressArchive(tarOut, findAppropriatePrefix(dest.getPath()), dest);
		tarOut.close();
	}

	static public void main(String[] args) throws Throwable
	{
		// future: could use our cool code above to handle any archive type they pass.
		if (args.length != 2) {
			System.err.println("USAGE: PackTar {tar.gz file} {source location}");
			System.exit(1);
		}

		try {
			PackTar.compressTarGZ(new File(args[0]), new File(args[1]));
		} catch (Throwable t) {
			_logger.error("failed to compress tar file " + args[0] + " from " + args[1]);
			System.exit(1);
		}

		System.out.println("successfully compressed archive file " + args[0] + " from " + args[1]);
	}

}
