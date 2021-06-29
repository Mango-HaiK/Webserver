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

## 11.分组数据
# GROUP BY 必须在WHERE子句之后，ORDER BY子句之前
# WHERE分组前进行过滤，而HAVING在分组后过滤，二者的操作符一样

# 创建分组
SELECT vend_id, COUNT(*) AS num_prods
FROM products
WHERE vend_id
GROUP BY vend_id;

# 过滤分组 
SELECT cust_id, COUNT(*) AS orders
FROM orders
GROUP BY cust_id
HAVING COUNT(*) >= 2;

# 先筛选掉价格大于10的，再列出数目大于2的
SELECT vend_id, COUNT(*) AS num_prods
FROM products
WHERE prod_price >= 10
GROUP BY vend_id
HAVING COUNT(*) >= 2;

SELECT order_num, SUM(quantity * item_price) AS ordertotal
FROM orderitems
GROUP BY order_num
HAVING SUM(quantity * item_price) >= 50
ORDER BY ordertotal;

## 12.子查询
SELECT cust_id
FROM orders
WHERE order_num IN (SELECT order_num
                    FROM orderitems
                    WHERE prod_id = 'TNT2');

SELECT cust_name, cust_contact
FROM customers
WHERE cust_id IN (SELECT cust_id
                  FROM orders
                  WHERE order_num IN (SELECT order_num
                                      FROM orderitems
                                      WHERE prod_id = 'TNT2'));

## 13.联结表
# 加上需要查找的表就行了
SELECT vend_name, prod_name, prod_price
FROM vendors, products
WHERE vendors.vend_id = products.vend_id
ORDER BY vend_name, prod_name;

# 没有联结关系将返回笛卡尔积（两个表第行数相乘）
SELECT vend_name, prod_name, prod_price
FROM vendors, products
ORDER BY vend_name, prod_name;

# 联结多个表
---联结的表越多性能越低
SELECT prod_name, vend_name, prod_price, quantity
FROM orderitems, products, vendors
WHERE products.vend_id = vendors.vend_id
  AND orderitems.prod_id = products.prod_id
  AND order_num = 20005;

SELECT cust_name, cust_contact
FROM customers, orders, orderitems
WHERE customers.cust_id = orders.cust_id
  AND orderitems.order_num = orders.order_num
  AND prod_id = 'TNT2';

## 14.创建高级联结
# 使用表的别名 - 表别名不返回客户机
SELECT cust_name, cust_contact
FROM customers AS c, orders AS o, orderitems AS oi
WHERE c.cust_id = o.cust_id
  AND oi.order_num = o.order_num
  AND prod_id = 'TNT2';

# 使用不同的类型联结
# -自联结
SELECT p1.prod_id, p1.prod_name
FROM products AS p1, products AS p2
WHERE p1.vend_id = p2.vend_id
  AND p2.prod_id = 'DTNTR';

# -自然联结
SELECT c.*, o.order_num, o.order_date,
        oi.prod_id, oi.quantity, oi.item_price
FROM customers AS c, orders AS o, orderitems AS oi
WHERE c.cust_id = o.cust_id
  AND oi.order_num = o.order_num
  AND prod_id = 'FB';

## 15.组合查询 - 通过UNION组合两个查询(也可使用WHERE组合)
SELECT vend_id, prod_id, prod_price
FROM products
WHERE prod_price <= 5
UNION
SELECT vend_id, prod_id, prod_price
FROM products
WHERE vend_id IN (1001, 1002);

# 使用UNION ALL可返回所有匹配的行，没有ALL将去重
# 也可对数据进行ORDER BY进行排序，但只能有一个
SELECT vend_id, prod_id, prod_price
FROM products
WHERE prod_price <= 5
UNION ALL 
SELECT vend_id, prod_id, prod_price
FROM products
WHERE vend_id IN (1001, 1002)
ORDER BY vend_id, prod_price;


## 16.全文本搜索
# **仅在MyISAM支持全文本搜索
# Match指定查找的列，Against指定搜索表达式,BIHARY区分大小写 
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('rabbit');

# 查询扩展，搜索表达式后加WITH QUERY EXPANSION
#两次查询，第一次找到所在的行，然后在该行检查有用的词(存在的词)
# 之后使用这些词进行第二次查询，返回匹配的行
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('anvils' WITH QUERY EXPANSION);

# 布尔文本搜索，搜索表达式后加 IN BOOLENAN MODE
# 没有FULLTEXT索引也可以使用
# 匹配heavy,但是如果出现rope则排除
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('heavy -rope*' IN BOOLEAN MODE);

/*
|布尔操作符|说明|
|----|----|
|+|包含，词必须存在|
|-|排除，词必须不出现|
|>|包含，而且增加等级值|
|<|包含，而且减少等级值|
|()|把词组成字表达式(允许这些子表达式作为一个组被包含、排除和排序等)|
|~|取消一个词的排序值|
|*|词尾的通配符|
|""|定义一个短语（与单个词的列表不一样，他匹配短语以便包含或排除这个短语）|
*/

SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('+rabbit +bait' IN BOOLEAN MODE);

# 至少出现rabbit和bait的一个
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('rabbit bait' IN BOOLEAN MODE);

# 匹配短语""，而不是单个词
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('"rabbit bait"' IN BOOLEAN MODE);

# 匹配safe和combination，降低后者等级
SELECT note_text
FROM productnotes
WHERE Match(note_text) Against('+safe + +(<combination)' IN BOOLEAN MODE);

## 17.插入数据
# 插入完整的行
INSERT INTO customers
VALUES (NULL,
        'Pep E. LaPew',
        '100 Main Street',
        'Los Angeles',
        'CA',
        '90046',
        'USA',
        NULL,
        NULL);

# 也可以这样，更安全，但也更繁琐,列名可不按表的顺序，但是值必须得对应
# 也可以省略列，但需要满足列可为NULL，或提供了默认值
INSERT INTO customers(cust_name,
        cust_address,
        cust_city,
        cust_state,
        cust_zip,
        cust_country,
        cust_contact,
        cust_email)
VALUES (NULL,
        'Pep E. LaPew',
        '100 Main Street',
        'Los Angeles',
        'CA',
        '90046',
        'USA',
        NULL,
        NULL);

# 插入多个行
INSERT INTO customers(cust_name,
      cust_address,
      cust_city,
      cust_state,
      cust_zip,
      cust_country
)
VALUES(
      'Pep E. LaPew',
      '100 Main Street',
      'Los Angeles',
      'CA',
      '90046',
      'USA'
),(
      'M. Martian',
      '42 Galaxy Way',
      'New York',
      'NY',
      '11213',
      'USA'
);

# *使用单个INSERT语句插入多个行速度会快些

# 插入检索出的数据
INSERT INTO customers(cust_name,
        cust_address,
        cust_city,
        cust_state,
        cust_zip,
        cust_country,
        cust_contact,
        cust_email)
VALUES(SELECT cust_name,
        cust_address,
        cust_city,
        cust_state,
        cust_zip,
        cust_country,
        cust_contact,
        cust_email
      FROM custnew);

# *INSERT语句也可以包含WHERE子句过滤插入的数据

## 18.更新和删除数据
# *不要省略WHERE子句，不然会更新整个表
UPDATE customers
SET cust_email = 'elmer@fudd.com'
WHERE cust_id = 10005;

# 更新多个列
UPDATE customers
SET cust_name = 'The Fudds',
    cust_email = 'elemer@fudd.com'
WHERE cust_id = 10005;

# *UPDATE语句也可使用子查询
# *IGNORE关键字，更新多行时指定可在出错时继续执行剩下的语句
# *用法：UPDATE IGNORE customers...

# 删除数据
# *务必记得使用WHERE不然会删除整个表的行
DELETE FROM customers
WHERE cust_id = 10006;

# *DELETE删除整行而不是列，删除列可使用UPDATE
# *TRUNCATE TABLE 语句删除原来的表再重新创建一个表(效率更快)
# *在使用UPDATE和DELETE时先使用SELETE进行测试，以保证过滤的是否正确

## 19.创建表和操纵表
# 表创建基础 - customers表的创建语句
CREATE TABLE customers /*IF NOT EXISTS （表名不存在时创建）*/
(
        cust_id      INT        NOT NULL AUTO_INCREMENT ,
        cust_name    char(50)   NOT NULL ,
        cust_address char(50)   NULL ,
        cust_city    char(50)   NULL ,
        cust_state   char(5)    NULL ,
        cust_zip     char(10)   NULL ,
        cust_country char(50)   NULL DEFAULT 'China',
        cust_contact char(50)   NULL ,
        cust_email   char(255)  NULL ,
        PRIMARY(cust_id)        /*设定主键*/
) ENGINE = InnoDB;

# *NULL指定可为空值，加上NOT则不可为空值
# *PRIMARY(order_id, order_num),多个主键可这样写
# *AUTO_INCREMENT,自动增量，每个表只允许一个
# *DEFAULT，指定默认值

# 引擎类型
/*
|引擎|说明|
|----|----|
|InnoDB|可靠的事务处理引擎，但不支持全文本搜索|
|MyISAM|性能极高的引擎，支持全文本搜索但不支持事务处理|
|MEMORY|等同于MyISAM，但由于数据存储在内存中，速度很快（适合与临时表）|
*/
# * InnoDB为MySQL的默认引擎
# * 引擎类型可以混用，但是外键不能跨引擎

# 更新表
#* 可使用ALTER TABLE修改表，但是在存储数据后就不应再被更新。
#* 给一个表增加一个名为vend_phone的列，必须明确其数据类型。
;
ALTER TABLE vendors
ADD vend_phone CHAR(20);

# *删除一个列
ALTER TABLE vendors
DROP COLUMN vend_phone;

# *定义外键
/*
ALTER TABLE <数据表名> ADD CONSTRAINT <外键名>
FOREIGN KEY (<列名>) REFERENCES <主表名>(<列名>)
*/
ALTER TABLE orderitems
ADD CONSTRAINT fk_orderitems_orders
FOREIGN KEY (order_num) REFERENCES orders (order_num);

# 删除表
# *如果存在则删除，没有确认，也不能撤销，永久删除
DROP TABLE customers2;

# 重命名表
# *可在后面加上  ,<表名> TO <新表名>进行多个表的重命名
RENAME TABLE customers2 TO customers;

## 20.视图
# *视图是对表的查询结果，索引并不存储数据
# *使用多个联结和过滤创建了复杂的视图会降低性能
# *和表一样视图必须唯一命名，创建的视图的数目没有限制
# *创建视图需要足够的访问权限
# *视图也可与嵌套
# *ORDER BY也可用在视图中，但如果该视图检索数据的SELECT语句中也包含有ORDER BY，那么该视图中的ORDER BY将会被覆盖
# *视图不能索引，也不能关联的触发器或默认值
# *视图可与表一起使用，如SELECT。

# 使用视图
# *视图使用CREATE VIEW 创建
# *SHOW CHREAT VIEW 查看创建视图的语句
# *DROP VIEW <viewname>,删除视图
# *更新视图可使用DROP再使用CREATE，也可直接使用CREAT REPLACE VIEW

# 利用视图简化复杂的联结
;
CREATE VIEW porductcustomers AS 
SELECT cust_name, cust_contact, prod_id
FROM customers, orders, orderitems
WHERE customers.cust_id = orders.cust_id
  AND orderitems.order_num = orders.order_num;
