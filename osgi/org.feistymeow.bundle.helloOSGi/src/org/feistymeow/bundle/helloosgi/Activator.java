package org.feistymeow.bundle.helloosgi;

import org.feistymeow.bundle.serviceosgi.HelloService;
import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;

@SuppressWarnings("rawtypes")
public class Activator implements BundleActivator {

	ServiceReference helloServiceReference;
    
	/*
	 * (non-Javadoc)
	 * @see org.osgi.framework.BundleActivator#start(org.osgi.framework.BundleContext)
	 */
	public void start(BundleContext context) throws Exception {
		
		System.out.println("into Hello OSGi!");
		
		helloServiceReference = context.getServiceReference(HelloService.class.getName());
        @SuppressWarnings("unchecked")
		HelloService helloService = (HelloService)context.getService(helloServiceReference);
        System.out.println(helloService.sayHello());
	}
	
	/*
	 * (non-Javadoc)
	 * @see org.osgi.framework.BundleActivator#stop(org.osgi.framework.BundleContext)
	 */
	public void stop(BundleContext context) throws Exception {
		System.out.println("out of Hello OSGi!");
	}

}
