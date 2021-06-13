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