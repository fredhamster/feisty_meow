package org.feistymeow.utility;

import java.io.File;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

//////////////
// Name   : Extemporizer
// Author : Chris Koeritz
// Rights : Copyright (c) 2012-$now By University of Virginia
//////////////
// This file is free software; you can modify/redistribute it under the terms
// of the Apache License v2.0: http://www.apache.org/licenses/LICENSE-2.0
// Feel free to send updates to: [ koeritz@virginia.edu ]
//////////////

/**
 * A set of useful utilities for dealing with temporary items.
 */
public class Extemporizer
{
    static private Log _logger = LogFactory.getLog(Extemporizer.class);

    /**
     * creates a uniquely named temporary directory. thanks for guidance to article at:
     * http://stackoverflow.com/questions/617414/create-a-temporary-directory-in-java
     * 
     * @return a File object pointing at the new temporary directory.
     * @throws IOException
     */
    public static File createTempDirectory(String prefix, String suffix)
    {
        if ((prefix == null) || (suffix == null))
            return null;
        try {
            final File temp = File.createTempFile(prefix, suffix);
            if (!temp.delete())
                throw new IOException("failed to delete temporary file: " + temp.getAbsolutePath());
            if (!temp.mkdir())
                throw new IOException("failed to create temporary directory: "
                        + temp.getAbsolutePath());
            temp.deleteOnExit(); // set for cleanup.
            return temp;
        } catch (Throwable cause) {
            _logger.error("caught exception while creating temporary directory", cause);
            return null;
        }
    }
}
