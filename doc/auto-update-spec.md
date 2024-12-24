# Auto Update Specification (Language: zh_CN)
## 自动更新规范

### 客户端的自动更新请求
1. 请求形式为 HTTP GET。
2. 支持 HTTPS。
3. User-Agent 为 "BTC Tools v1.0" 这样的格式
4. 客户端会附加扩展头信息 X-App-Version-Id 和 X-App-Version-Name 用于服务器端识别其版本。

### 服务器的自动更新响应
1. 响应数据为JSON类型
2. 格式如下：
```json
{
    "minVersionId": 1,
    "versionId": 2,
    "versionName": "v1.1",
    "desc": "this is a test update\ntest update\ntest update",
    "desc_zh_CN": "这是一个测试更新\n测试更新\n测试更新",
    "downloadUrl": "https://pool.btc.com/tools"
}
```
3. `minVersionId`为允许运行的最小版本号，小于该版本号的客户端将会要求强制更新。
4. `versionId`为当前最新的版本号，`versionName`为对应的版本名。
5. `desc_zh_CN`为中文更新说明，其他语言的更新说明为`desc_对应的语言`，如`desc_en_US`。
6. 如果没有对应语言代码的更新说明，客户端使用`desc`做为其更新说明。目前建议`desc`中的内容使用英语。
