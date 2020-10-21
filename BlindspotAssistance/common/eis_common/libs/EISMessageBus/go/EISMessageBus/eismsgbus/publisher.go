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
	msgbus "EISMessageBus/internal/pkg/msgbus"
	"errors"
)

type Publisher struct {
	client *MsgbusClient
	pubCtx *msgbus.PublisherContext
}

func newPublisher(msgbusClient *MsgbusClient, pubCtx *msgbus.PublisherContext) *Publisher {
	pub := new(Publisher)
	pub.client = msgbusClient
	pub.pubCtx = pubCtx
	return pub
}

func (pub *Publisher) Publish(msg interface{}) error {
	if pub.client.IsClosed() {
		return errors.New("Message bus context has been destroyed")
	}
	if pub.pubCtx == nil {
		return errors.New("Publisher already closed")
	}
	return pub.client.ctx.Publish(pub.pubCtx, msg)
}

func (pub *Publisher) Close() error {
	if pub.client.IsClosed() {
		return errors.New("Message bus context has been destroyed")
	}
	pub.client.ctx.DestroyPublisher(pub.pubCtx)
	pub.pubCtx = nil
	return nil
}
