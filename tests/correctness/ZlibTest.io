Zlib

uncompressed := "TestSuite clone setPath(System launchPath) run"
testFolder := File with(Path thisSourceFilePath) parentDirectory path
zCompressedFile := File with(Path with(testFolder, "test.z"))
zCompressed := method(zCompressedFile contents)
gzCompressedFile := File with(Path with(testFolder, "test.gz"))
gzCompressed := method(gzCompressedFile contents)

ZlibTest := UnitTest clone do(
    testRoundTrip := method(
        compressed := Zlib compress(Sequence clone appendSeq(uncompressed))
        actual := Zlib uncompress(Sequence clone appendSeq(compressed))
        assertEquals(actual, uncompressed)
    )
)

ZlibDecoderTest := UnitTest clone do(
    testGzBasic := method(
        z := ZlibDecoder clone
        z setInputBuffer(Sequence clone appendSeq(gzCompressed))
        # z outputBuffer := Sequence clone
        z beginProcessing
        z process
        z endProcessing

        assertEquals(z outputBuffer, uncompressed)
    )

    testGzDecompress := method(
        actual := Zlib uncompress(gzCompressed)

        assertEquals(actual, uncompressed)
    )

    testGzUnzip := method(
        actual := Sequence clone appendSeq(gzCompressed) unzip

        assertEquals(actual, uncompressed)
    )

    testBasic := method(
        z := ZlibDecoder clone
        z setInputBuffer(Sequence clone appendSeq(zCompressed))
        z outputBuffer := Sequence clone
        z beginProcessing
        z process
        z endProcessing

        assertEquals(z outputBuffer, uncompressed)
    )

    testDecompress := method(
        actual := Zlib uncompress(zCompressed)

        assertEquals(actual, uncompressed)
    )

    testUnzip := method(
        actual := Sequence clone appendSeq(zCompressed) unzip

        assertEquals(actual, uncompressed)
    )
)

ZlibEncoderTest := UnitTest clone do(
    testBasic := method(
        z := ZlibEncoder clone
        z beginProcessing
        z inputBuffer appendSeq(uncompressed)
        z process
        z endProcessing

        assertEquals(z outputBuffer, zCompressed)
    )

    testCompress := method(
        actual := Zlib compress(Sequence clone appendSeq(uncompressed))

        assertEquals(actual, zCompressed)
    )
)
