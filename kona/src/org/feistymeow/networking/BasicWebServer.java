package org.feistymeow.networking;

import java.util.*;
import java.io.*;
import java.net.*;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Provides a lightweight way for RNS structures to be accessible over http.
 * 
 * @Author Chris Koeritz
 */

/*
 * original example thanks to Matt Mahoney, at
 * http://cs.fit.edu/~mmahoney/cse3103/java/Webserver.java
 */

public class BasicWebServer
{
	static private Log logger = LogFactory.getLog(BasicWebServer.class);

	private int port;
	private boolean leaving = false; // turns to true when should stop serving.
	servingThread socketThread;
	private ServerSocket realSocket;

	BasicWebServer(int portIn)
	{
		port = portIn;
	}

	public void shutDown()
	{
		leaving = true;
		if (realSocket != null) {
			try {
				realSocket.close();
			} catch (IOException e) {
			}
		}
		if (socketThread != null) {
			// stop it?
		}
	}

	public class servingThread implements Runnable
	{
		private Thread thread;
		private ServerSocket serverSocket;

		servingThread(ServerSocket socket)
		{
			serverSocket = socket;
			thread = new Thread(this);
			thread.start();
		}

		@Override
		public void run()
		{
			while (!leaving) {
				try {
					logger.debug("about to accept on server socket.");
					Socket s = serverSocket.accept(); // Wait for a client to
														// connect
					logger.debug("accepted client, spawning handler.");
					new ClientHandler(s); // Handle the client in a separate
											// thread
				} catch (Throwable cause) {
					logger.error("exception raised while handling accepted socket", cause);
				}
			}
		}
	}

	// enums for outcomes? really need better reporting.
	public int startServing()
	{
		if (socketThread != null)
			return 1; // already running outcome.
		try {
			realSocket = new ServerSocket(port);
		} catch (Throwable cause) {
			logger.error("failure to start server on port " + port, cause);
			return 1;
			// socket failure outcome.
		}
		socketThread = new servingThread(realSocket);
		return 0;
	}

	public String predictMimeType(String filename)
	{

		// kludge to try one type:
		return "text/plain;charset=utf-8";

		/*
		 * 
		 * if (filename.endsWith(".html") || filename.endsWith(".htm")) return "text/html"; if
		 * (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg"; if
		 * (filename.endsWith(".gif")) return "image/gif"; if (filename.endsWith(".class")) return
		 * "application/octet-stream"; return "text/plain";
		 */
	}

	// A ClientHandler reads an HTTP request and responds
	class ClientHandler extends Thread
	{
		private Socket socket; // The accepted socket from the Webserver

		// Start the thread in the constructor
		public ClientHandler(Socket s)
		{
			socket = s;
			start();
		}

		// Read the HTTP request, respond, and close the connection
		public void run()
		{
			try {
				logger.debug("into client run(): listening for gets.");

				// Open connections to the socket
				BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
				PrintStream out = new PrintStream(new BufferedOutputStream(socket.getOutputStream()));

				// Read filename from first input line "GET /filename.html ..."
				// or if not in this format, treat as a file not found.
				String s = in.readLine();
				logger.debug("request is: " + s); // Log the request

				// Attempt to serve the file. Catch FileNotFoundException and
				// return an HTTP error "404 Not Found". Treat invalid requests
				// the same way.
				String filename = "";
				StringTokenizer st = new StringTokenizer(s);
				try {

					boolean transferFile = true;
					// get the command first.
					String command = st.nextToken();
					// Parse the filename from the command.
					if (st.hasMoreElements() && command.equalsIgnoreCase("GET") && st.hasMoreElements()) {
						filename = st.nextToken();
					} else if (st.hasMoreElements() && command.equalsIgnoreCase("HEAD") && st.hasMoreElements()) {
						filename = st.nextToken();
						transferFile = false; // don't need to do that, just the
												// header.
					} else {
						logger.error("going to blow file not found exception now.");
						throw new FileNotFoundException(); // Bad request
					}
					logger.info("filename to handle is now: " + filename);

					// Append trailing "/" with "index.html"
					// /hmmm: may want to make this assume directory.
					if (filename.endsWith("/"))
						logger.error("unhandled attempt to get item ending in slash");
					// if (filename.endsWith("/"))
					// filename += "index.html";

					// Remove leading / from filename
					// / while (filename.indexOf("/") == 0)
					// / filename = filename.substring(1);

					// Replace "/" with "\" in path for PC-based servers
					filename = filename.replace('/', File.separator.charAt(0));

					logger.info("asking for rns path of " + filename);

					// Check for illegal characters to prevent access to
					// superdirectories
					if (filename.indexOf("..") >= 0 || filename.indexOf(':') >= 0 || filename.indexOf('|') >= 0)
						throw new FileNotFoundException();

					logger.info("got past filename checks for: " + filename);

					/*
					 * this doesn't actually check that trailing slash is missing! // If a directory
					 * is requested and the trailing / is missing, // send the client an HTTP
					 * request to append it. (This is // necessary for relative links to work
					 * correctly in the client). if ((new GeniiPath(filename)).isDirectory()) {
					 * out.print("HTTP/1.0 301 Moved Permanently\r\n" + "Location: /" + filename +
					 * "/\r\n\r\n"); out.close(); return; }
					 */

					// trying to get around worrying about mime types by saying
					// "just get this there".
					String mimeType = predictMimeType(filename);
					// //"application/octet-stream";

					File source = new File(filename);
					if (!source.exists()) {
						logger.error("source does not exist for serving: " + filename);
						// do something!
						// hmmm: below could be abstracted to more general
						// denial method.
						out.println("HTTP/1.1 404 Not Found\r\n" + "Content-type: text/html\r\n\r\n"
							+ "<html><head></head><body>" + filename + " not found</body></html>\n");
						out.close();

					}
					out.print("HTTP/1.1 200 OK\r\n" + "Content-type: " + mimeType + "\r\n" + "Connection: close" + "\r\n"
					// // + "\r\nContent-Length: " + source.size? +
					// "\r\n"
						+ "\r\n");
					if (!transferFile) {
						logger.debug("closing stream for finished HEAD request.");
						out.close();
						return;
					}
					logger.debug("moving to handle GET request.");
					FileInputStream f = new FileInputStream(filename);
					logger.debug("opened stream on source");
					// Send file contents to client, then close the connection.
					byte[] a = new byte[4096];
					int n;
					while ((n = f.read(a)) > 0)
						out.write(a, 0, n);
					logger.debug("wrote file back for request, closing stream.");
					out.close();
				} catch (FileNotFoundException x) {
					logger.error("failed to find requested file: " + filename);
					out.println("HTTP/1.1 404 Not Found\r\n" + "Content-type: text/html\r\n\r\n" + "<html><head></head><body>"
						+ filename + " not found</body></html>\n");
					out.close();
				}
			} catch (IOException x) {
				logger.error("exception blew out in outer area of web server", x);
				// / System.out.println(x);
			}
		}
	}
}
