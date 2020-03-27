#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "UnpooledHeapCharBuf.hpp"
#include "CommUtil.hpp"
#include "UnpooledSlicedCharBuf.hpp"
#include "PooledCharBufAllocator.hpp"

using namespace std;
using namespace comm::buffer;

TEST(UnpooledHeapCharBufTest, func1)
{
	UnpooledHeapCharBuf buf;
	string str = "This is a test message";//22
	buf.writeBool(true);
	buf.writeChar('a');// 1
	buf.writeInt( str.length() ); // 2
	buf.writeString( str ); // 6
	buf.writeShort(0x7fff); // 28
	buf.writeUShort(0xffff); // 30 
	buf.writeInt(0x7fffffff);// 32 
	buf.writeUInt(0xffffffff); // 36
	buf.writeLong(65535); // 40
	buf.writeULong(6553565535);//48
	buf.writeFloat(31.31); // 56
	buf.writeDouble(99.12345678); // 60 + 8
	buf.writeInt( str.length() );//68 + 4
	buf.writeChars(str.c_str(), str.length());//72 + 22
	UnpooledHeapCharBuf buf2 = buf;
	
	EXPECT_TRUE(buf.writerIndex() == 94);
	EXPECT_TRUE(buf.getBool(0) == true);
	EXPECT_TRUE(buf.getLong(40) == 65535);
	EXPECT_TRUE(buf.getULong(48) == 6553565535);
	EXPECT_TRUE(buf.getChar(1) == 'a');
	string str2;
	EXPECT_TRUE(buf.readBool() == true);
	EXPECT_TRUE(buf.readChar() == 'a');
	int len = buf.readInt();
	EXPECT_TRUE( len == str.length() );
	buf.readString(str2, len);
	EXPECT_TRUE(str == str2);
	EXPECT_TRUE(buf.readShort() == 0x7fff);
	EXPECT_TRUE(buf.readUShort() == 0xffff);
	EXPECT_TRUE(buf.readInt() == 0x7fffffff);
	EXPECT_TRUE(buf.readUInt() == 0xffffffff);
	EXPECT_TRUE(buf.readLong() == 65535);
	EXPECT_TRUE(buf.readULong() == 6553565535);
	float f = buf.readFloat() ;
	EXPECT_TRUE( f -  31.31 <= 0.0 );
	double d = buf.readDouble();
	EXPECT_TRUE( d - 99.12345678 <= 0.0 );
	EXPECT_TRUE( buf.readerIndex() == 68 );

	cout << buf.readerIndex() << " f:" <<f << " d:" << d  << endl;
	cout << "capacity:" <<  buf.capacity() << " maxCapacity:" << buf.maxCapacity() << endl;


	buf2.setBool(0, false);
	buf2.setChar(1, 'b');// 1
	//buf2.setInt( str.length() ); // 2
	buf2.setString(6, "ABCD" ); // 6
	buf2.setShort(28, 1); // 28
	buf2.setUShort(30, 2); // 30 
	buf2.setInt(32, 3);// 32 
	buf2.setUInt(36, 4); // 36
	buf2.setLong(40, 5); // 40
	buf2.setULong(48, 6);//48
	buf2.setFloat(56, 32.32); // 56
	buf2.setDouble(60, 100.100); // 60 + 8
	EXPECT_TRUE(buf2.writerIndex() == 94);
	EXPECT_TRUE(buf2.getBool(0) == false);
	EXPECT_TRUE(buf2.getLong(40) == 5);
	EXPECT_TRUE(buf2.getULong(48) == 6);
	EXPECT_TRUE(buf2.readBool() == false);
	EXPECT_TRUE(buf2.readChar() == 'b');
	len = buf2.readInt();
	EXPECT_TRUE( len == str.length() );
	buf2.readString(str2, len);
	EXPECT_TRUE(str2 == "ABCD is a test message");
	EXPECT_TRUE(buf2.readShort() == 1);
	EXPECT_TRUE(buf2.readUShort() == 2);
	EXPECT_TRUE(buf2.readInt() == 3);
	EXPECT_TRUE(buf2.readUInt() == 4);
	EXPECT_TRUE(buf2.readLong() == 5);
	EXPECT_TRUE(buf2.readULong() == 6);
	f = buf2.readFloat() ;
	EXPECT_TRUE( f -  32.32 <= 0.0 );
	d = buf2.readDouble();
	EXPECT_TRUE( d - 100.100 <= 0.0 );
	EXPECT_TRUE( buf2.readerIndex() == 68 );
}

TEST(UnpooledHeapCharBufTest, func2)
{
	UnpooledHeapCharBuf buf;
	
	string str(81890, 'a');//22
	buf.writeInt( str.length() ); // 2
	buf.writeString( str );

	UnpooledHeapCharBuf buf2 = buf;

	int len = buf.readInt();
	string str2;
	buf.readString(str2, len);
	EXPECT_TRUE(len == str.length());
	EXPECT_TRUE(str == str2);
	cout << "capacity:" <<  buf.capacity() << " maxCapacity:" << buf.maxCapacity() << endl;
	cout << CommUtil::roundUpSize(8179) << endl;
	cout << CommUtil::roundUpSize(8172) << endl;
	cout << CommUtil::roundUpSize(8193) << endl;
	cout << CommUtil::roundUpPower2(8193) << endl;
	//  function
	len = buf2.readInt();
	str2.clear();
	buf2.readString(str2, len);
	EXPECT_TRUE(len == str.length());
	EXPECT_TRUE(str == str2);
	EXPECT_TRUE(buf.capacity()== buf2.capacity());
	EXPECT_TRUE(buf.maxCapacity()== buf2.maxCapacity());

	UnpooledHeapCharBuf buf3;
	buf.writeInt( str.length() );
	buf.writeString( str );
	buf3.writeChars(buf);
	str2.clear();
	len = buf3.readInt();
	buf3.readString(str2, len);
	EXPECT_TRUE(len == str.length());
	EXPECT_TRUE(str == str2);
}

TEST(UnpooledHeapCharBufTest, sliceTest)
{
	boost::shared_ptr<UnpooledHeapCharBuf> buf(new UnpooledHeapCharBuf());
	string str = "This is a test message";//22
	buf->writeBool(true);
	buf->writeChar('a');// 1
	buf->writeInt( str.length() ); // 2
	buf->writeString( str ); // 6
	EXPECT_TRUE(buf->refCnt() == 1);

	boost::shared_ptr<CharBuf> sliceBuffer = buf->retainedSlice(2, 4 + str.length());
	EXPECT_TRUE(buf->refCnt() == 2);

	string str2;
	int len = sliceBuffer->getInt(0);
	EXPECT_TRUE(len == str.length());
	sliceBuffer->getString(4, str2, len);
	EXPECT_TRUE(str == str2);

	sliceBuffer->setString(4, "ABCD");
	len = sliceBuffer->readInt();
	EXPECT_TRUE(len == str.length());
	sliceBuffer->readString(str2, len);
	EXPECT_TRUE("ABCD is a test message" == str2);

	EXPECT_TRUE(buf->readBool() == true);
	EXPECT_TRUE(buf->readChar() == 'a');
	len = buf->readInt();
	EXPECT_TRUE( len == str.length() );
	buf->readString(str2, len);
	EXPECT_TRUE("ABCD is a test message" == str2);

	sliceBuffer->release();
	EXPECT_TRUE(buf->refCnt() == 1);
	sliceBuffer.reset();
}

TEST(PooledCharBufAllocator, TestLargeSize)
{
	CharBufAllocator* allocator =  CharBufAllocator::DEFAULT_ALLOCATOR;
	int num = 10240;
	while (num-- >0) {
		boost::shared_ptr<CharBuf> buf = allocator->heapBuffer(8200);
		string str = "This is a test message";//22
		buf->writeBool(true);
		buf->writeChar('a');// 1
		buf->writeInt( str.length() ); // 2
		buf->writeString( str ); // 6
		EXPECT_TRUE(buf->refCnt() == 1);

		boost::shared_ptr<CharBuf> sliceBuffer = buf->retainedSlice(2, 4 + str.length());
		EXPECT_TRUE(buf->refCnt() == 2);

		string str2;
		int len = sliceBuffer->getInt(0);
		EXPECT_TRUE(len == str.length());
		sliceBuffer->getString(4, str2, len);
		EXPECT_TRUE(str == str2);

		sliceBuffer->setString(4, "ABCD");
		len = sliceBuffer->readInt();
		EXPECT_TRUE(len == str.length());
		sliceBuffer->readString(str2, len);
		EXPECT_TRUE("ABCD is a test message" == str2);

		EXPECT_TRUE(buf->readBool() == true);
		EXPECT_TRUE(buf->readChar() == 'a');
		len = buf->readInt();
		EXPECT_TRUE( len == str.length() );
		buf->readString(str2, len);
		EXPECT_TRUE("ABCD is a test message" == str2);

		sliceBuffer->release();
		EXPECT_TRUE(buf->refCnt() == 1);
		sliceBuffer.reset();
	}
}

TEST(PooledCharBufAllocator, TestTinySize)
{
	CharBufAllocator* allocator =  CharBufAllocator::DEFAULT_ALLOCATOR;
	int num = 10240;
	while (num-- >0) {
		boost::shared_ptr<CharBuf> buf = allocator->heapBuffer(1026);
		string str = "This is a test message";//22
		buf->writeBool(true);
		buf->writeChar('a');// 1
		buf->writeInt( str.length() ); // 2
		buf->writeString( str ); // 6
		EXPECT_TRUE(buf->refCnt() == 1);

		boost::shared_ptr<CharBuf> sliceBuffer = buf->retainedSlice(2, 4 + str.length());
		EXPECT_TRUE(buf->refCnt() == 2);

		string str2;
		int len = sliceBuffer->getInt(0);
		EXPECT_TRUE(len == str.length());
		sliceBuffer->getString(4, str2, len);
		EXPECT_TRUE(str == str2);

		sliceBuffer->setString(4, "ABCD");
		len = sliceBuffer->readInt();
		EXPECT_TRUE(len == str.length());
		sliceBuffer->readString(str2, len);
		EXPECT_TRUE("ABCD is a test message" == str2);

		EXPECT_TRUE(buf->readBool() == true);
		EXPECT_TRUE(buf->readChar() == 'a');
		len = buf->readInt();
		EXPECT_TRUE( len == str.length() );
		buf->readString(str2, len);
		EXPECT_TRUE("ABCD is a test message" == str2);

		sliceBuffer->release();
		EXPECT_TRUE(buf->refCnt() == 1);
		sliceBuffer.reset();

		buf->release();
	}
}