/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package eismsgbus

import (
	types "EISMessageBus/pkg/types"
)

// Subscriber object. This object is not thread-safe.
type Subscriber struct {
	MessageChannel chan *types.MsgEnvelope
	ErrorChannel   chan error
	quitChannel    chan interface{}
	closed         bool
}

// Creates a new subscriber structure.
func newSubscriber(msgCh chan *types.MsgEnvelope, quitCh chan interface{}, errCh chan error) *Subscriber {
	sub := new(Subscriber)

	sub.MessageChannel = msgCh
	sub.ErrorChannel = errCh
	sub.quitChannel = quitCh
	sub.closed = false

	return sub
}

// Close the subscriber
func (subscriber *Subscriber) Close() {
	if subscriber.closed {
		return
	}

	// Signal gorouting to stop
	close(subscriber.MessageChannel)
	close(subscriber.ErrorChannel)
	close(subscriber.quitChannel)
	subscriber.closed = true
}
