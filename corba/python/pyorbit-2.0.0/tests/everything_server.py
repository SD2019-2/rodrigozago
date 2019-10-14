import sys
import ORBit

ORBit.load_typelib('Everything')

import CORBA
import orbit.test
import orbit__POA.test
import constants

class MyBasicServer(orbit__POA.test.BasicServer):
    def _get_foo(self):
        return constants.STRING_RETN
    def _set_foo(self, value):
        assert value == constants.STRING_IN
    def _get_bah(self):
        return constants.LONG_RETN

    def opString(self, inArg, inoutArg):
        assert inArg == constants.STRING_IN
        assert inoutArg == constants.STRING_INOUT_IN
        return (constants.STRING_RETN,
                constants.STRING_INOUT_OUT,
                constants.STRING_OUT)

    def opLong(self, inArg, inoutArg):
        assert inArg == constants.LONG_IN
        assert inoutArg == constants.LONG_INOUT_IN
        return (constants.LONG_RETN,
                constants.LONG_INOUT_OUT,
                constants.LONG_OUT)

    def opLongLong(self, inArg, inoutArg):
        assert inArg == constants.LONG_LONG_IN
        assert inoutArg == constants.LONG_LONG_INOUT_IN
        return (constants.LONG_LONG_RETN,
                constants.LONG_LONG_INOUT_OUT,
                constants.LONG_LONG_OUT)

    def opFloat(self, inArg, inoutArg):
        assert abs(inArg - constants.FLOAT_IN) < 0.00005
        assert abs(inoutArg - constants.FLOAT_INOUT_IN) < 0.00005
        return (constants.FLOAT_RETN,
                constants.FLOAT_INOUT_OUT,
                constants.FLOAT_OUT)

    def opDouble(self, inArg, inoutArg):
        assert inArg == constants.FLOAT_IN
        assert inoutArg == constants.FLOAT_INOUT_IN
        return (constants.FLOAT_RETN,
                constants.FLOAT_INOUT_OUT,
                constants.FLOAT_OUT)

    def opEnum(self, inArg, inoutArg):
        assert inArg == orbit.test.ENUM_IN
        assert inoutArg == orbit.test.ENUM_INOUT_IN
        return (orbit.test.ENUM_RETN,
                orbit.test.ENUM_INOUT_OUT,
                orbit.test.ENUM_OUT)

    def opException(self):
        raise orbit.test.TestException(constants.STRING_IN,
                                       constants.LONG_IN,
                                       [ constants.LONG_IN ],
                                       self._this())

    def opOneWay(self, inArg):
        assert inArg == constants.STRING_IN

class MyStructServer(orbit__POA.test.StructServer, MyBasicServer):
    def opFixed(self, inArg, inoutArg):
        assert inArg.a == constants.SHORT_IN
        assert inoutArg.a == constants.SHORT_INOUT_IN
        return (orbit.test.FixedLengthStruct(constants.SHORT_RETN),
                orbit.test.FixedLengthStruct(constants.SHORT_INOUT_OUT),
                orbit.test.FixedLengthStruct(constants.SHORT_OUT))

    def opVariable(self, inArg, inoutArg):
        assert inArg.a == constants.STRING_IN
        assert inoutArg.a == constants.STRING_INOUT_IN
        return (orbit.test.FixedLengthStruct(constants.STRING_RETN),
                orbit.test.FixedLengthStruct(constants.STRING_INOUT_OUT),
                orbit.test.FixedLengthStruct(constants.STRING_OUT))
    
    def opCompound(self, inArg, inoutArg):
        assert inArg.a.a == constants.STRING_IN
        assert inoutArg.a.a == constants.STRING_INOUT_IN
        return (orbit.test.CompoundStruct(orbit.test.FixedLengthStruct(constants.STRING_RETN)),
                orbit.test.CompoundStruct(orbit.test.FixedLengthStruct(constants.STRING_INOUT_OUT)),
                orbit.test.CompoundStruct(orbit.test.FixedLengthStruct(constants.STRING_OUT)))

    def opObjectStruct(self, inArg):
        assert inArg.serv == self._this()

    def opStructAny(self):
        tc = CORBA.TypeCode('IDL:omg.org/CORBA/long:1.0')
        return orbit.test.StructAny(constants.STRING_IN,
                                    CORBA.Any(tc, constants.LONG_IN))

class MySequenceServer(orbit__POA.test.SequenceServer):
    def opStrSeq(self, inArg, inoutArg):
        for i in range(len(inArg)):
            assert inArg[i] == constants.SEQ_STRING_IN[i]
        for i in range(len(inoutArg)):
            assert inoutArg[i] == constants.SEQ_STRING_INOUT_IN[i]
        return (constants.SEQ_STRING_RETN,
                constants.SEQ_STRING_INOUT_OUT,
                constants.SEQ_STRING_OUT)
    def opBoundedStructSeq(self, inArg, inoutArg):
        for i in range(len(inArg)):
            assert inArg[i].a.a == constants.SEQ_STRING_IN[i]
        for i in range(len(inoutArg)):
            assert inoutArg[i].a.a == constants.SEQ_STRING_INOUT_IN[i]
        retn = [
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_RETN[0])),
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_RETN[1]))
            ]
        inout = [
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_INOUT_OUT[0])),
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_INOUT_OUT[1]))
            ]
        out = [
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_OUT[0])),
            orbit.test.CompoundStruct(orbit.test.VariableLengthStruct(constants.SEQ_STRING_OUT[1]))
            ]
        return retn, inout, out

class MyUnionServer(orbit__POA.test.UnionServer):
    def opFixed(self, inArg, inoutArg):
        assert inArg.x == constants.LONG_IN
        assert inoutArg.y == 't'
        return (orbit.test.FixedLengthUnion('d', CORBA.FALSE),
                orbit.test.FixedLengthUnion('c', CORBA.TRUE),
                orbit.test.FixedLengthUnion('a', constants.LONG_OUT))
    def opVariable(self, inArg, inoutArg):
        assert inArg.x == constants.LONG_IN
        assert inoutArg.y == constants.STRING_INOUT_IN
        return (orbit.test.VariableLengthUnion(4, CORBA.FALSE),
                orbit.test.VariableLengthUnion(3, CORBA.TRUE),
                orbit.test.VariableLengthUnion(1, constants.LONG_OUT))
    def opMisc(self, inSeq, inArg):
        assert len(inSeq) == 3
        assert inSeq[0]._d == 4
        assert inSeq[0]._v == CORBA.TRUE
        assert inSeq[1].y == "blah"
        assert inSeq[2]._d == 55
        assert inSeq[2].w == constants.LONG_IN
        assert inArg.y == "blah de blah"
        return (orbit.test.EnumUnion(orbit.test.EnumUnion.red,
                                     constants.LONG_IN),
                orbit.test.ArrayUnion(22,
                                      [ 'Numero %d' % i for i in range(20) ]))

class MyArrayServer(orbit__POA.test.ArrayServer):
    def opLongArray(self, inArg, inoutArg):
        for i in range(len(inArg)):
            assert inArg[i] == constants.SEQ_LONG_IN[i]
        for i in range(len(inoutArg)):
            assert inoutArg[i] == constants.SEQ_LONG_INOUT_IN[i]
        return (constants.SEQ_LONG_RETN,
                constants.SEQ_LONG_INOUT_OUT,
                constants.SEQ_LONG_OUT)

    def opStrArray(self, inArg, inoutArg):
        for i in range(len(inArg)):
            assert inArg[i] == constants.SEQ_STRING_IN[i]
        for i in range(len(inoutArg)):
            assert inoutArg[i] == constants.SEQ_STRING_INOUT_IN[i]
        return (constants.SEQ_STRING_RETN,
                constants.SEQ_STRING_INOUT_OUT,
                constants.SEQ_STRING_OUT)

class MyAnyServer(orbit__POA.test.AnyServer):
    def opAnyStrSeq(self):
        tc = CORBA.TypeCode('IDL:orbit/test/StrSeq:1.0')
        return CORBA.Any(tc, [ 'foo' ] * 16)

    def opAnyLong(self, inArg, inoutArg):
        tc = CORBA.TypeCode('IDL:omg.org/CORBA/long:1.0')
        assert inArg.typecode() == tc
        assert inArg.value() == constants.LONG_IN
        assert inoutArg.typecode() == tc
        assert inoutArg.value() == constants.LONG_INOUT_IN
        return (CORBA.Any(tc, constants.LONG_RETN),
                CORBA.Any(tc, constants.LONG_INOUT_OUT),
                CORBA.Any(tc, constants.LONG_OUT))

    def opAnyString(self, inArg, inoutArg):
        tc = CORBA.TypeCode('IDL:omg.org/CORBA/string:1.0')
        assert inArg.typecode() == tc
        assert inArg.value() == constants.STRING_IN
        assert inoutArg.typecode() == tc
        assert inoutArg.value() == constants.STRING_INOUT_IN
        return (CORBA.Any(tc, constants.STRING_RETN),
                CORBA.Any(tc, constants.STRING_INOUT_OUT),
                CORBA.Any(tc, constants.STRING_OUT))

    def opAnyStruct(self, inArg, inoutArg):
        tc = CORBA.TypeCode('IDL:orbit/test/VariableLengthStruct:1.0')
        assert inArg.typecode() == tc
        assert inArg.value().a == constants.STRING_IN
        assert inoutArg.typecode() == tc
        assert inoutArg.value().a == constants.STRING_INOUT_IN
        return (CORBA.Any(tc, orbit.test.VariableLengthStruct(constants.STRING_RETN)),
                CORBA.Any(tc, orbit.test.VariableLengthStruct(constants.STRING_INOUT_OUT)),
                CORBA.Any(tc, orbit.test.VariableLengthStruct(constants.STRING_OUT)))

    def opTypeCode(self, inArg, inoutArg):
        assert inArg == CORBA.TypeCode('IDL:orbit/test/ArrayUnion:1.0')
        assert inoutArg == CORBA.TypeCode('IDL:orbit/test/AnyServer:1.0')
        return (CORBA.TypeCode('IDL:orbit/test/VariableLengthStruct:1.0'),
                CORBA.TypeCode('IDL:orbit/test/TestException:1.0'),
                CORBA.TypeCode('IDL:orbit/test/AnEnum:1.0'))


class MyFactory(orbit__POA.test.TestFactory):
    def __init__(self):
        orbit__POA.test.TestFactory.__init__(self)
        self.basicServer = MyBasicServer()
        self.structServer = MyStructServer()
        self.sequenceServer = MySequenceServer()
        self.unionServer = MyUnionServer()
        self.arrayServer = MyArrayServer()
        self.anyServer = MyAnyServer()
    def getBasicServer(self):
        return self.basicServer._this()
    def getStructServer(self):
        return self.structServer._this()
    def getSequenceServer(self):
        return self.sequenceServer._this()
    def getUnionServer(self):
        return self.unionServer._this()
    def getArrayServer(self):
        return self.arrayServer._this()
    def getAnyServer(self):
        return self.anyServer._this()

if __name__ == '__main__':
    orb = CORBA.ORB_init(sys.argv)

    factory = MyFactory()
    objref = factory._this()
    file('iorfile', 'w').write(orb.object_to_string(objref))

    factory._default_POA().the_POAManager.activate()
    orb.run()
