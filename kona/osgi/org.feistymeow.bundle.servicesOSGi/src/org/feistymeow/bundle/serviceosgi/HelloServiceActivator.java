package org.feistymeow.bundle.serviceosgi;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceRegistration;

public class HelloServiceActivator implements BundleActivator
{

	@SuppressWarnings("rawtypes")
	ServiceRegistration helloServiceRegistration;

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.osgi.framework.BundleActivator#start(org.osgi.framework.BundleContext)
	 */
	public void start(BundleContext context) throws Exception
	{
		System.out.println("hello-service start");

		HelloService helloService = new HelloServiceImpl();
		helloServiceRegistration = context.registerService(HelloService.class.getName(), helloService, null);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.osgi.framework.BundleActivator#stop(org.osgi.framework.BundleContext)
	 */
	public void stop(BundleContext context) throws Exception
	{
		helloServiceRegistration.unregister();
		System.out.println("hello-service stop");
	}

}
