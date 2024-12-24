# Miner Type/Stat Specification (Language: zh_CN)
## 矿机类型/状态规范

### 扫描结果规范
扫描器（btctools::miner::MinerScanner）返回的 Miner 结构体应该满足以下条件：
1. 若扫描过程没有发生错误，Miner.stat_ 应为 "success"
2. 若扫描过程中发生错误，Miner.stat_ 中应包含简单的错误信息
3. 若扫描过程中发生错误，Miner.typeStr_ 应为 "unknown"
4. 若扫描过程中没有发生错误，但是未识别出矿机类型，Miner.typeStr_ 应为 "unknown"
5. 若识别出矿机类型，Miner.typeStr_ 应为小写的矿机类型名称（配置器要能正确识别该名称）

### 配置结果规范
配置器（btctools::miner::MinerConfigurator）返回的 Miner 结构体应该满足以下条件：
1. 不改变 Miner.ip_ 、 Miner.typeStr_ 、 Miner.fullTypeStr_
2. 若配置过程发生错误，Miner.stat_ 中应包含简单的错误信息
3. 若配置过程没有发生错误，并且矿机明确响应“配置成功”，则 Miner.stat_ 应为 "ok"
4. 若配置过程没有发生错误，但是矿机没有明确响应“配置成功”，则 Miner.stat_ 不能为 "ok"，而是设置为相关的错误信息
5. 不应将 Miner.stat_ 设置为 "success"，成功时应设置为 "ok"
