package org.gffs.io;

import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;

import org.apache.log4j.Logger;

public class IOUtils
{
	static private Logger _logger = Logger.getLogger(IOUtils.class);

	static final private int BUFFER_SIZE = 1024 * 8;

	static private final int COPY_SIZE = 1024 * 8;

	static public void copy(InputStream in, OutputStream out) throws IOException
	{
		byte[] data = new byte[BUFFER_SIZE];
		int read;

		while ((read = in.read(data)) > 0)
			out.write(data, 0, read);
	}

	static public void close(Closeable closeable)
	{
		if (closeable != null) {
			try {
				closeable.close();
			} catch (Throwable cause) {
				_logger.error("Error trying to close closeable item.", cause);
			}
		}
	}

	static public void recursiveDelete(File target)
	{
		if (!target.exists())
			return;

		if (target.isDirectory())
			for (File newTarget : target.listFiles())
				recursiveDelete(newTarget);

		target.delete();
	}

	static public void copy(File source, File target) throws IOException
	{
		InputStream in = null;
		OutputStream out = null;

		try {
			in = new FileInputStream(source);
			out = new FileOutputStream(target);
			copy(in, out);
		} finally {
			close(in);
			close(out);
		}
	}

	static public void serialize(String filePath, Object obj) throws IOException
	{
		serialize(new File(filePath), obj);
	}

	static public void serialize(File target, Object obj) throws IOException
	{
		FileOutputStream fos = null;

		try {
			fos = new FileOutputStream(target);
			ObjectOutputStream oos = new ObjectOutputStream(fos);
			oos.writeObject(obj);
			oos.close();
		} finally {
			close(fos);
		}
	}

	static public <Type> Type deserialize(Class<Type> cl, String sourcePath) throws FileNotFoundException, IOException
	{
		return deserialize(cl, new File(sourcePath));
	}

	static public <Type> Type deserialize(Class<Type> cl, File source) throws FileNotFoundException, IOException
	{
		FileInputStream fin = null;

		try {
			fin = new FileInputStream(source);
			ObjectInputStream ois = new ObjectInputStream(fin);
			return cl.cast(ois.readObject());
		} catch (ClassNotFoundException e) {
			throw new IOException("Unable to deserialize from file.", e);
		} finally {
			close(fin);
		}
	}
}
