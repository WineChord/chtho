// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/net/Buffer.h"

#include <string> 

using namespace chtho;
using namespace chtho::net;

void testBufferAppendRetrieve()
{
  Buffer buf;
  assert(buf.readableBytes() == 0);
  assert(buf.writableBytes() == Buffer::kInit);
  assert(buf.prependableBytes() == Buffer::kPre);

  const std::string s(200, 'x');
  buf.append(s);
  assert(buf.readableBytes() == s.size());
  assert(buf.writableBytes() == Buffer::kInit-s.size());
  assert(buf.prependableBytes() == Buffer::kPre);

  const std::string s2 =  buf.retrieveAsString(50);
  assert(s2.size() == 50);
  assert(buf.readableBytes() == s.size() - s2.size());
  assert(buf.writableBytes() == Buffer::kInit-s.size());
  assert(buf.prependableBytes() == Buffer::kPre+s2.size());
  assert(s2 == std::string(50, 'x'));

  buf.append(s);
  assert(buf.readableBytes() == 2*s.size()-s2.size());
  assert(buf.writableBytes() == Buffer::kInit-2*s.size());
  assert(buf.prependableBytes() == Buffer::kPre+s2.size());

  const std::string s3 = buf.retrieveAllAsString(); 
  assert(s3.size() == 350);
  assert(buf.readableBytes() == 0);
  assert(buf.writableBytes() == Buffer::kInit);
  assert(buf.prependableBytes() == Buffer::kPre);
  assert(s3 == std::string(350, 'x'));
}

void testBufferGrow()
{
  Buffer buf;
  buf.append(std::string(400, 'y'));
  assert(buf.readableBytes() == 400);
  assert(buf.writableBytes() == Buffer::kInit-400);

  buf.retrieve(50);
  assert(buf.readableBytes() == 350);
  assert(buf.writableBytes() == Buffer::kInit-400);
  assert(buf.prependableBytes() == Buffer::kPre+50);

  buf.append(std::string(1000, 'z'));
  assert(buf.readableBytes() == 1350);
  assert(buf.writableBytes() == 0);
  assert(buf.prependableBytes() == Buffer::kPre+50);

  buf.retrieveAll();
  assert(buf.readableBytes() == 0);
  assert(buf.writableBytes() == 1400);
  assert(buf.prependableBytes() == Buffer::kPre);
}

void testBufferInsideGrow()
{
  Buffer buf;
  buf.append(std::string(800, 'y'));
  assert(buf.readableBytes() == 800);
  assert(buf.writableBytes() == Buffer::kInit-800);

  buf.retrieve(500);
  assert(buf.readableBytes() == 300);
  assert(buf.writableBytes() == Buffer::kInit-800);
  assert(buf.prependableBytes() == Buffer::kPre+500);

  buf.append(std::string(300, 'z'));
  assert(buf.readableBytes() == 600);
  assert(buf.writableBytes() == Buffer::kInit-600);
  assert(buf.prependableBytes() == Buffer::kPre);
}

void testBufferShrink()
{
  Buffer buf;
  buf.append(std::string(2000, 'y'));
  assert(buf.readableBytes() == 2000);
  assert(buf.writableBytes() == 0);
  assert(buf.prependableBytes() == Buffer::kPre);

  buf.retrieve(1500);
  assert(buf.readableBytes() == 500);
  assert(buf.writableBytes() == 0);
  assert(buf.prependableBytes() == Buffer::kPre+1500);

  buf.shrink(0);
  assert(buf.readableBytes() == 500);
  assert(buf.writableBytes() == Buffer::kInit-500);
  assert(buf.retrieveAllAsString() == std::string(500, 'y'));
  assert(buf.prependableBytes() == Buffer::kPre);
}

void testBufferPrepend()
{
  Buffer buf;
  buf.append(std::string(200, 'y'));
  assert(buf.readableBytes() == 200);
  assert(buf.writableBytes() == Buffer::kInit-200);
  assert(buf.prependableBytes() == Buffer::kPre);

  int x = 0;
  buf.prepend(&x, sizeof(x));
  assert(buf.readableBytes() == 204);
  assert(buf.writableBytes() == Buffer::kInit-200);
  assert(buf.prependableBytes() == Buffer::kPre-4);
}

void testBufferReadInt()
{
  Buffer buf;
  buf.append("HTTP");
  assert(buf.readableBytes() == 4);
  assert(buf.peekInt8() == 'H');
  int top16 = buf.peekInt16();
  assert(top16 == 'H'*256+'T');
  assert(buf.peekInt32() == top16*65536+'T'*256+'P');

  assert(buf.readInt8() == 'H');
  assert(buf.readInt16() == 'T'*256+'T');
  assert(buf.readInt8() == 'P');
  assert(buf.readableBytes() == 0);
  assert(buf.writableBytes() == Buffer::kInit);

  buf.appendInt8(-1);
  buf.appendInt16(-2);
  buf.appendInt32(-3);
  assert(buf.readableBytes() == 7);
  assert(buf.readInt8() == -1);
  assert(buf.readInt16() == -2);
  assert(buf.readInt32() == -3);
}

void testBufferFindEOL()
{
  Buffer buf;
  buf.append(std::string(100000, 'x'));
  const char* null = NULL;
  assert(buf.findEOL() == null);
  assert(buf.findEOL(buf.peek()+90000) == null);
}

void output(Buffer&& buf, const void* inner)
{
  Buffer n(std::move(buf));
  assert(inner == n.peek());
}

void testMove()
{
  Buffer buf;
  buf.append("chtho", 5);
  const void* inner = buf.peek();
  output(std::move(buf), inner);
}

int main()
{
  testBufferAppendRetrieve();
  testBufferGrow();
  testBufferInsideGrow();
  testBufferShrink();
  testBufferPrepend();
  testBufferReadInt();
  testBufferFindEOL();
  testMove();
}