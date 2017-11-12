package org.gffs.network;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;

import org.gffs.io.IOUtils;

public abstract class URLDownloader
{
	static final private int CONNECTION_TIMEOUT = 1000 * 8;
	static final private int READ_TIMEOUT = 1000 * 8;

	static public InputStream connect(URL url) throws IOException
	{
		URLConnection connection = url.openConnection();
		connection.setConnectTimeout(CONNECTION_TIMEOUT);
		connection.setReadTimeout(READ_TIMEOUT);
		connection.connect();
		return connection.getInputStream();
	}

	static public void download(URL source, File target) throws IOException
	{
		InputStream in = null;
		OutputStream out = null;

		try {
			in = connect(source);
			out = new FileOutputStream(target);
			IOUtils.copy(in, out);
		} finally {
			IOUtils.close(in);
			IOUtils.close(out);
		}
	}
}