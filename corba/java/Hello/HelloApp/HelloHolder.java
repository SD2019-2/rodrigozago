package HelloApp;

/**
* HelloApp/HelloHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.1"
* from Hello.idl
* Domingo, 13 de Outubro de 2019 07h52min28s BRT
*/

public final class HelloHolder implements org.omg.CORBA.portable.Streamable
{
  public HelloApp.Hello value = null;

  public HelloHolder ()
  {
  }

  public HelloHolder (HelloApp.Hello initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = HelloApp.HelloHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    HelloApp.HelloHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return HelloApp.HelloHelper.type ();
  }

}
