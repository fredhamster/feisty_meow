package org.feistymeow.bundle.serviceosgi;

public class HelloServiceImpl implements HelloService
{

	@Override
	public String sayHello()
	{
		System.out.println("Inside HelloServiceImpl.sayHello()");
        return "Say Hello";
	}

}
