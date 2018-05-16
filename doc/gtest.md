
# TEST
```TEST(测试集名称, 测试用例名称)```

一个测试集内可以有多个测试用例，gtest是按照测试集为单元进行执行，一个测试集内的全部测试用例执行完成后再执行下一个测试集的用例。
通常，将一个类型的测试用例组织到一个测试集(test case)中。同一个测试集中的不用用例测试不同条件下的情况。


# TEST_F 
```
class AddTest : public testing::Test
{
public:
	virtual void SetUp()    { puts("SetUp()"); }
	virtual void TearDown() { puts("TearDown()"); }
};

TEST_F(AddTest, test1) {
	puts("test TEST_F1");


	ASSERT_GT(add(1, 2), 3); // 故意的  
	ASSERT_EQ(add(4, 5), 6); // 也是故意的  
}

TEST_F(AddTest, test2) {
	puts("test TEST_F1");


	ASSERT_GT(add(1, 2), 3); // 故意的  
	ASSERT_EQ(add(4, 5), 6); // 也是故意的  
}
```
TEST_F 测试方式需要自己先定义一个class，TEST_F后第一个参数就是class name，第二个参数测试用例名称。
在class中有两个特殊的虚函数 SetUp和TearDown，如果有实现分别在每个用例执行前和执行完成后会被自动调用。可以用来构建测试环境，和一些清理工作。同时，自定义的class中还可以
在实现一些自定义的函数，用来辅助验证。


# 测试用例的过滤设置
--gtest_filter=  在vc工程配置中增加运行参数，-表示排查 :表示或


