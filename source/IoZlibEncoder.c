//metadoc ZlibEncoder copyright Steve Dekorte, 2004
//metadoc ZlibEncoder license BSD revised
//metadoc ZlibEncoder category API
/*metadoc ZlibEncoder description
For <a href=http://en.wikipedia.org/wiki/Zlib>Zlib</a> compression.
Example use:
<p>
<pre>	
z := ZlibEncoder clone
z beginProcessing
z inputBuffer appendSeq("this is a message")
z process
z endProcessing
result := z outputBuffer
</pre>	
*/

//doc ZlibEncoder inputBuffer The input buffer for decoding.
//doc ZlibEncoder outputBuffer The output buffer for decoding.

#include "IoZlibEncoder.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoSeq.h"

#define DATA(self) ((IoZlibEncoderData *)(IoObject_dataPointer(self)))
static const char *protoId = "ZlibEncoder";

IoTag *IoZlibEncoder_newTag(void *state)
{
	IoTag *tag = IoTag_newWithName_(protoId);
	IoTag_state_(tag, state);
	IoTag_freeFunc_(tag, (IoTagFreeFunc *)IoZlibEncoder_free);
	IoTag_cloneFunc_(tag, (IoTagCloneFunc *)IoZlibEncoder_rawClone);
	return tag;
}

IoZlibEncoder *IoZlibEncoder_proto(void *state)
{
	IoZlibEncoder *self = IoObject_new(state);
	IoObject_tag_(self, IoZlibEncoder_newTag(state));

	IoObject_setDataPointer_(self, calloc(1, sizeof(IoZlibEncoderData)));
	DATA(self)->strm = calloc(1, sizeof(z_stream));
	DATA(self)->level = 9;

	IoState_registerProtoWithId_(state, self, protoId);

	{
		IoMethodTable methodTable[] = {
		{"beginProcessing", IoZlibEncoder_beginProcessing},
		{"process", IoZlibEncoder_process},
		{"endProcessing", IoZlibEncoder_endProcessing},
		{NULL, NULL},
		};
		IoObject_addMethodTable_(self, methodTable);
	}

	return self;
}

IoZlibEncoder *IoZlibEncoder_rawClone(IoZlibEncoder *proto)
{
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, calloc(1, sizeof(IoZlibEncoderData)));
	DATA(self)->strm = calloc(1, sizeof(z_stream));
	return self;
}

IoZlibEncoder *IoZlibEncoder_new(void *state)
{
	IoObject *proto = IoState_protoWithId_(state, protoId);
	return IOCLONE(proto);
}

void IoZlibEncoder_free(IoZlibEncoder *self)
{
	free(DATA(self));
}

// -----------------------------------------------------------

IoObject *IoZlibEncoder_beginProcessing(IoZlibEncoder *self, IoObject *locals, IoMessage *m)
{
	/*doc ZlibEncoder beginProcessing
	Initializes the algorithm.
	*/
	
	z_stream *strm = DATA(self)->strm;
	int ret;

	strm->zalloc = Z_NULL;
	strm->zfree = Z_NULL;
	strm->opaque = Z_NULL;
	strm->avail_in = 0;
	strm->next_in = Z_NULL;
	//ret = deflateInit(strm, DATA(self)->level); // zlib format
	ret = deflateInit2(strm, DATA(self)->level, Z_DEFLATED, 16 + 15, 8, Z_DEFAULT_STRATEGY); // gz format
	IOASSERT(ret == Z_OK, "unable to initialize zlib via deflateInit()");

	return self;
}

IoObject *IoZlibEncoder_endProcessing(IoZlibEncoder *self, IoObject *locals, IoMessage *m)
{
	/*doc ZlibEncoder endProcessing
	Finish processing remaining bytes of inputBuffer.
	*/
	
	z_stream *strm = DATA(self)->strm;
	int ret;

	IoZlibEncoder_process(self, locals, m);
	ret = deflateEnd(strm);
	IOASSERT(ret == Z_OK, "unable to finish zlib via deflateEnd()");

	DATA(self)->isDone = 1;
	return self;
}

IoObject *IoZlibEncoder_process(IoObject *self, IoObject *locals, IoMessage *m)
{
	/*doc ZlibEncoder process
	Process the inputBuffer and appends the result to the outputBuffer.
	The processed inputBuffer is emptied except for the spare bytes at the end which don't fit into a cipher block.
	*/
	
	z_stream *strm = DATA(self)->strm;

	UArray *input  = IoObject_rawGetMutableUArraySlot(self, locals, m, IOSYMBOL("inputBuffer"));
	UArray *output = IoObject_rawGetMutableUArraySlot(self, locals, m, IOSYMBOL("outputBuffer"));

	uint8_t *inputBytes = (uint8_t *)UArray_bytes(input);
	size_t inputSize = UArray_sizeInBytes(input);

	if (inputSize)
	{
		int ret;
		size_t oldOutputSize = UArray_size(output);
		size_t outputRoom = (inputSize * 2);
		uint8_t *outputBytes;

		UArray_setSize_(output, oldOutputSize + outputRoom);
		outputBytes = (uint8_t *)UArray_bytes(output) + oldOutputSize;

		strm->next_in   = inputBytes;
		strm->avail_in  = inputSize;

		strm->next_out  = outputBytes;
		strm->avail_out = outputRoom;

		ret = deflate(strm, Z_FINISH); // compress in one go

		//assert(ret != Z_STREAM_ERROR);
		//assert(ret == Z_OK); // progress has been made (more input processed or more output produced)
		IOASSERT(ret == Z_STREAM_END, "unable to process zlib via deflate()");
		//assert(strm->avail_in == 0); // if > 0, must be called again after making room in the output buffer
		//assert(strm->avail_out > 0); // if == 0, must be called again with the same value of the flush parameter and more output space
		{
		size_t outputSize = outputRoom - strm->avail_out;
		UArray_setSize_(output, oldOutputSize + outputSize);
		}

		UArray_setSize_(input, 0);
	}

	return self;
}

