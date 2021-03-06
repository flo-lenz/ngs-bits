#include "TestFramework.h"

TEST_CLASS(SampleDiff_Test)
{
Q_OBJECT
private slots:
	
	void test_01()
	{
		EXECUTE_FAIL("SampleDiff", "-in1 " + TESTDATA("data_in/SampleDiff_in1.tsv") + " -in2 " + TESTDATA("data_in/SampleDiff_in2.tsv") + " -out out/SampleDiff_out1.txt");
		COMPARE_FILES("out/SampleDiff_out1.txt", TESTDATA("data_out/SampleDiff_out1.txt"));
	}
	
	void test_02()
	{
		EXECUTE_FAIL("SampleDiff", "-in1 " + TESTDATA("data_in/SampleDiff_in1.tsv") + " -in2 " + TESTDATA("data_in/SampleDiff_in2.tsv") + " -out out/SampleDiff_out2.txt -sm");
		COMPARE_FILES("out/SampleDiff_out2.txt", TESTDATA("data_out/SampleDiff_out2.txt"));
	}
	
	void test_03()
	{
		EXECUTE_FAIL("SampleDiff", "-in1 " + TESTDATA("data_in/SampleDiff_in1.tsv") + " -in2 " + TESTDATA("data_in/SampleDiff_in2.tsv") + " -out out/SampleDiff_out3.txt -ei");
		COMPARE_FILES("out/SampleDiff_out3.txt", TESTDATA("data_out/SampleDiff_out3.txt"));
	}

};

