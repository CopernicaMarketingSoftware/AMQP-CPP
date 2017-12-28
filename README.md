这是我Fork的一个项目，我是用在RabbitMQ上，在与RabbitMQ-C互通的时候，解析包时总出错，后来才发现field.cpp中，解析frame的方法中，有一个‘x’域的解析没有，而RabbitMQ-C有应该是为了兼容QPID吧，所以我Fork了一下，添加了这样的语句。
