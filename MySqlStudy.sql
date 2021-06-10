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