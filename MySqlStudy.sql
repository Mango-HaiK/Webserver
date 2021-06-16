## 1.了解数据库和表
# 选择数据库
USE mydb;

#显示数据库
SHOW DATABASES;

#显示表
SHOW TABLES;

# 给出表名显示可用列
SHOW COLUMNS FROM customers;

## 2.检索数据
# 检索单个列
SELECT prod_name FROM products;

# 检索多个列
SELECT prod_id, prod_name, prod_price
FROM products;

# 检索所有列
SELECT * FROM products;

# 检索不同的行,使用关键字DISTICNCT指示只返回不同的值
SELECT DISTINCT vend_id
FROM products;

# 限制结果,LIMIT指示返回多少行，从0开始计数
SELECT prod_name
FROM products
LIMIT 5;

# 从第5行开始的5行
SELECT prod_name
FROM products
LIMIT 5,5; 

## 3.排序检索数据
# 会根据ORDER BY后面的列名进行字母排序
SELECT prod_name
FROM products
ORDER BY prod_name;

# 按多个列排序
# 先按ORDER BY后的第一个列名排序，如果相同才按第二个排，以此类推
SELECT prod_id, prod_price, prod_name
FROM products
ORDER BY prod_price, prod_name;

# 指定排序方向
# 使用关键字DESC可按倒序排序，之后还可再加列名
SELECT prod_id, prod_price, prod_name
FROM products
ORDER BY prod_price DESC;

# 使用ORDER BY和LIMIT组合可找出最值
SELECT prod_price
FROM products
ORDER BY prod_price DESC
LIMIT 1;

## 4.过滤数据
# 使用WHRERE子句过滤
SELECT prod_name, prod_price
FROM products
WHERE prod_price = 2.50;

# 检查单个值
SELECT prod_name, prod_price
FROM products
WHERE prod_name = 'safe';

# 也可使用操作符
SELECT prod_name, prod_price
FROM  products
WHERE prod_price > 5;

# BETWEEN 取两个值之间
SELECT  prod_name, prod_price
FROM products
WHERE prod_price BETWEEN 5 AND 10;

# IS NULL 关键字检测空值
SELECT cust_id
FROM  customers
WHERE cust_email IS NULL ;

# 5.数据过滤
# 可使用多个WHERE子句，并通过 AND 和 OR连接
SELECT prod_id, prod_price, prod_name,
FROM  products
WHERE vend_id = 1003 AND prod_price <= 10;

SELECT vend_id, prod_name, prod_price
FROM products
WHERE vend_id = 1002 OR vend_id = 1003;

# 使用()明确分组
SELECT vend_id, prod_name, prod_price
FROM products
WHERE (vend_id = 1002 OR vend_id = 1003) AND prod_price >= 10;

# IN 操作符用于指定条件范围（与OR具有相同功能但优点多）
SELECT prod_name, prod_price
FROM products
WHERE vend_id IN (1002,1003)
ORDER BY prod_name;

# NOT 操作符，否定后跟但所有条件
SELECT prod_name, prod_price
FROM products
WHERE vend_id NOT IN (1002,1003)
ORDER BY prod_name;

# 6.用通配符进行过滤
# LIKE-后跟搜索模式(通配符和字面值组合)进行匹配搜索,
# 尽量不要使通配符放在开头会影响效率
# '%'-表示任意字符出现任意次数
SELECT prod_id, prod_name
FROM products
WHERE prod_name LIKE '% ton %';

# '_'-只匹配单个字符
SELECT prod_id, prod_name
FROM products
WHERE prod_name LIKE '_ ton anvil';

## 7.使用正则表达式
# LIKE 和 REGEXP的区别
# LIKE匹配整个值 而REGEXP匹配子串

# 基本字符匹配 - REGEXP后面跟的是正则表达式
SELECT prod_name
FROM products
WHERE prod_name REGEXP '1000'
ORDER BY prod_name;

# ‘.’表示匹配单个数字
SELECT prod_name
FROM products
WHERE prod_name REGEXP '.000'
ORDER BY prod_name;

# OR 匹配
SELECT prod_name
FROM products
WHERE prod_name REGEXP '1000|2000'
ORDER BY prod_name;

# 匹配几个字符串之一 - 通过‘[]’指定
SELECT prod_name
FROM products
WHERE prod_name REGEXP '[123] Ton'
ORDER BY prod_name;

# 匹配范围[0-9]、[a-z]
SELECT prod_name
FROM products
WHERE prod_name REGEXP '[1-5] Ton'
ORDER BY prod_name;

# 匹配特殊字符，使用\\为前导进行转义
SELECT vend_name
FROM vendors
WHERE vend_name REGEXP '\\.'
ORDER BY vend_name;

# 匹配字符类
/* 
|类|说明|
|----|----|
|[:alnum:]|任意字母和数字|
|[:alpha:]|任意字符|
|[:blank:]|空格和制表符|
|[:cntrl:]|ASCII空置字符，0-31和127|
|[:graph:]|与[::print]相同，但不包括空格|
|[:digit:]|任意数字|
|[:lower:]|任意小写字母|
|[:upper:]|任意大写字母|
*/

# 匹配多个实例
/*
|元字符|说明|
|:----:|:----:|
|*|0个或多个匹配|
|+|1个或多个匹配(等同于{1,})|
|?|{0个或1个匹配(等同于{0,1})|
|{n}|指定数目的匹配|
|{n,}|不少于指定数目的匹配|
|{n,m}|匹配数目的范围(m不超过255)|
*/;
SELECT prod_name
FROM products
WHERE prod_name REGEXP '[:digit:]{4}'
ORDER BY prod_name;

# 定位符，匹配特殊位置的文本
/*
|元字符|说明|
|:----:|:----:|
|^|文本的开始|
|$|文本的结尾|
|[[:<:]]|词的开始|
|[[:>:]]|词的结尾|
*/
SELECT prod_name
FROM products
WHERE prod_name REGEXP '^[0-9\\.]'
ORDER BY prod_name;

## 8.创建计算字段
# 拼接字段-Concat()
SELECT CONCAT(vend_name,'(',vend_country,')')
FROM vendors
ORDER BY vend_name;

# 去掉空格Trim、RTrim、LTrim
SELECT CONCAT(RTrim(vend_name),'(',RTrim(vend_country),')')
FROM vendors
ORDER BY vend_name;

# 使用别名-AS
SELECT CONCAT(RTrim(vend_name),'(',RTrim(vend_country),')')
AS vend_title
FROM vendors
ORDER BY vend_name;

# 执行算术计算
SELECT prod_id,
       quantity,
       item_price,
       quantity * item_price AS expanded_price
FROM orderitems
WHERE order_num = 20005;

## 9.使用数据处理函数
/*常用的文本处理函数
｜函数｜说明｜
|----|----|
|Left()|返回串左边的字符|
|Length()|返回串的长度|
|Locate()|找出一个子串|
|Lower()|将串转换为小写|
|LTrim()|去掉左边空格|
|Right()|返回串右边的字符|
|RTrim()|去掉右边的空格|
|Soundex()|返回串的SUOUNDEX值(返回类似发音的串)|
|SubString()|返回字串的字符|
|Upper()|将串转换为大写|
*/;
SELECT vend_name ,Upper(vend_name) AS vend_name_upcase
FROM vendors
ORDER BY vend_name;

/*日期和时间处理函数
|函数|说明|
|----|----|
|AddDate()|添加一个日期（天、周）|
|AddTime()|添加一个时间（时、分）|
|CurDate()|返回当前日期|
|CurTime()|返回当前时间|
|Date()|返回日期时间的日期部分|
|DateDiff()|计算两个日期之差|
|Date_Add()|高度灵活的日期运算函数|
|Date_Format()|返回一个格式化的时间或日期串|
|Day()|返回日期的天数部分|
|DayOfWeek()|返回日期对应的星期几|
|Hour()|返回时间的小时部分|
|Minute()|返回时间的分钟部分|
|Month()|返回日期的月份部分|
|Now()|返回当前日期和时间|
|Second()|返回日期的秒部分|
|Time()|返回日期时间的时间部分|
|Year()|返回日期的年份部分|
*/;
# 应该使用函数提取时间
SELECT cust_id,order_num
FROM orders
WHERE Date(order_date) = '2005-09-01';

# 九月的订单
SELECT cust_id, order_num
FROM orders
WHERE Year(order_date) = 2005 AND Month(order_date) = 9;

/*数值处理函数
|函数|说明|
|Abs()|返回一个数的绝对值|
|Cos()|返回一个角度的余弦|
|Exp()|返回一个数的指数值|
|Mod()|返回除操作的余数|
|Pi()|返回圆周率|
|Rand()|返回一个随机数|
|Sin()|返回一个角度的正弦|
|Sqrt()|返回一个数的平方根|
|Tan()|返回一个角度的正切|
*/

## 10.汇总数据
/*SQL聚集数据
|函数|说明|
|----|----|
|AVG()|返回某列的平均值|
|COUNT()|返回某列的行数|
|MAX()|返回某列的最大值|
|MIN()|返回某列的最小值|
|SUM()|返回某列之和|
*/;
# AVG()
SELECT AVG(prod_price) AS avg_price
FROM products;

# COUNT()
SELECT COUNT(*) AS num_cust
FROM customers;

# MAX()
SELECT MAX(prod_price) AS max_price
FROM products;

# MIN()
SELECT MIN(prod_price) AS min_price
FROM products;

# SUM()
SELECT SUM(quantity) AS items_ordered
FROM orderitems
WHERE order_num = 20005;

SELECT SUM(quantity*item_price) AS total_price
FROM orderitems
WHERE order_num = 20005;

# 聚集不同的值
SELECT AVG(DISTINCT prod_price) AS avg_price
FROM products
WHERE vend_id = 1003;

# 组合聚集函数
SELECT COUNT(*) AS num_items,
        MIN(prod_price) AS price_min,
        MAX(prod_price) AS price_max,
        AVG(prod_price) AS price_avg
FROM products;