#include "TestFramework.h"
#include "QFileInfo"
#include <stdlib.h>

TEST_CLASS(BamDeduplicateByBarcode_Test)
{
Q_OBJECT
private slots:

	void test_bam_without_duplicates()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_in.fastq.gz") + " -test -out out/BamDeduplicateByBarcode_out.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicateByBarcode_out.bam"));
	}

	void test_bam_with_many_duplicates_same_barcode()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in2.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_in2.fastq") + " -test -out out/BamDeduplicateByBarcode_out2.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicateByBarcode_out2.bam"));
	}

	void test_bam_with_many_duplicates_and_unique_barcodes()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in2.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_in3.fastq") + " -test -out out/BamDeduplicateByBarcode_out3.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicateByBarcode_out2.bam"));
	}

	void test_compare_deduplicated_with_same_barcode_to_unique_barcodes()
	{
		QFileInfo same(TESTDATA("data_in/BamDeduplicateByBarcode_in2.bam"));
		QFileInfo unique("out/BamDeduplicateByBarcode_out3.bam");
		IS_TRUE((unique.size()-same.size())>8000);
	}

	void test_duplicates_regarding_mip_file()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -test -mip_file " + TESTDATA("data_in/FastqExtractBarcode_in_mips1.txt") +" -out out/BamDeduplicateByBarcode_out4.bam -stats out/BamDeduplicateByBarcode_out4.tsv");
		COMPARE_FILES("out/BamDeduplicateByBarcode_out4.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out4.tsv"));
	}

	void test_duplicates_regarding_mip_file_no_match()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -test -mip_file " + TESTDATA("data_in/FastqExtractBarcode_in_mips1.txt") +" -out out/BamDeduplicateByBarcode_out5.bam -nomatch_out out/BamDeduplicateByBarcode_no_match_out5.bed -stats out/BamDeduplicateByBarcode_out5.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_no_match_out5.bed -out out/BamDeduplicateByBarcode_no_match_out5_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out5.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out4.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_no_match_out5_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_no_match_out5_sorted.bed"));
	}

	void test_duplicates_regarding_mip_file_duplicates()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -test -mip_file " + TESTDATA("data_in/FastqExtractBarcode_in_mips1.txt") +" -out out/BamDeduplicateByBarcode_out6.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out6.bed -stats out/BamDeduplicateByBarcode_out6.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out6.bed -out out/BamDeduplicateByBarcode_duplicate_out6_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out6.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out4.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out6_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out6_sorted.bed"));
	}

	void test_duplicates_regarding_hs_file_duplicates()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in7.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in7.fastq.gz") + " -test -hs_file " + TESTDATA("data_in/FastqBamDeduplicate_in_hs1.bed") +" -out out/BamDeduplicateByBarcode_out7.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out7.bed -nomatch_out out/BamDeduplicateByBarcode_no_match_out7.bed -stats out/BamDeduplicateByBarcode_out7.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out7.bed -out out/BamDeduplicateByBarcode_duplicate_out7_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out7.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out7.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out7_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out7_sorted.bed"));
	}

	void test_duplicates_regarding_mip_file_reduce_singles()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -test -dist 1 -mip_file " + TESTDATA("data_in/FastqExtractBarcode_in_mips1.txt") +" -out out/BamDeduplicateByBarcode_out8.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out8.bed -stats out/BamDeduplicateByBarcode_out8.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out8.bed -out out/BamDeduplicateByBarcode_duplicate_out8_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out8.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out8.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out8_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out8_sorted.bed"));
	}

	void test_duplicates_regarding_hs_file_reduce_singles()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in7.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in7.fastq.gz") + " -test -dist 1 -hs_file " + TESTDATA("data_in/FastqBamDeduplicate_in_hs1.bed") +" -out out/BamDeduplicateByBarcode_out9.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out9.bed -nomatch_out out/BamDeduplicateByBarcode_no_match_out9.bed -stats out/BamDeduplicateByBarcode_out9.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out9.bed -out out/BamDeduplicateByBarcode_duplicate_out9_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out9.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out9.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out9_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out9_sorted.bed"));
	}

	void test_duplicates_regarding_hs_file_reduce_singles_min_group10()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in7.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in7.fastq.gz") + " -test -dist 100 -hs_file " + TESTDATA("data_in/FastqBamDeduplicate_in_hs1.bed") +" -min_group 10 -out out/BamDeduplicateByBarcode_out10.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out10.bed -nomatch_out out/BamDeduplicateByBarcode_no_match_out10.bed -stats out/BamDeduplicateByBarcode_out10.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out10.bed -out out/BamDeduplicateByBarcode_duplicate_out10_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out10.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out10.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out10_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out10_sorted.bed"));
	}


	void test_duplicates_regarding_mip_file_reduce_singles_min_group2()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -test -dist 1 -mip_file " + TESTDATA("data_in/FastqExtractBarcode_in_mips1.txt") +" -min_group 2 -out out/BamDeduplicateByBarcode_out11.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out11.bed -stats out/BamDeduplicateByBarcode_out11.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out11.bed -out out/BamDeduplicateByBarcode_duplicate_out11_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out11.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out11.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out11_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out8_sorted.bed"));
	}

	void test_duplicates_regarding_hs_file2_duplicates()
	{
		EXECUTE("BamDeduplicateByBarcode", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in7.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in7.fastq.gz") + " -test -hs_file " + TESTDATA("data_in/FastqBamDeduplicate_in_hs2.bed") +" -out out/BamDeduplicateByBarcode_out12.bam -duplicate_out out/BamDeduplicateByBarcode_duplicate_out12.bed -nomatch_out out/BamDeduplicateByBarcode_no_match_out12.bed -stats out/BamDeduplicateByBarcode_out12.tsv");
		EXECUTE("BedSort", "-in out/BamDeduplicateByBarcode_duplicate_out12.bed -out out/BamDeduplicateByBarcode_duplicate_out12_sorted.bed")
		COMPARE_FILES("out/BamDeduplicateByBarcode_out12.tsv", TESTDATA("data_out/BamDeduplicateByBarcode_out12.tsv"));
		COMPARE_FILES("out/BamDeduplicateByBarcode_duplicate_out12_sorted.bed", TESTDATA("data_out/BamDeduplicateByBarcode_duplicate_out12_sorted.bed"));
	}

};
