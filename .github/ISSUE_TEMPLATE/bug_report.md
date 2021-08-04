---
name: Bug report
about: Create a report to help us improve
title: ''
labels: ''
assignees: ''

---

**Describe the bug**
A clear and concise description of what the bug is.

**Expected behavior and actual behavior**
A clear and concise description of what you expected to happen.

**Sample code**
Provide a small piece of C++ code to demonstrate the issue. Watch out: do not use threads! AMQP-CPP objects are not thread-safe _by design_. If you construct channels or call methods from one thread, while the event loop is running in a different thread, unexpected things can and will happen.
```c++
int main()
{
    // add your example program without using threads
}
```
