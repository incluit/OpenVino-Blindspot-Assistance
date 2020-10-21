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
	types "EISMessageBus/pkg/types"
	"errors"
)

// ServiceRequester object. This object is not thread-safe.
//
// Since service request objects are not safe, all sending of requests and receiving
// of responses must occur in the same thread (i.e. goroutine).
type ServiceRequester struct {
	client  *MsgbusClient
	servCtx *msgbus.ReceiveContext
}

// Creates a new service structure.
func newServiceRequester(msgbusClient *MsgbusClient, servCtx *msgbus.ReceiveContext) *ServiceRequester {
	service := new(ServiceRequester)
	service.client = msgbusClient
	service.servCtx = servCtx
	return service
}

// Receive a response to a previously sent request.
//
// The timeout parameter determines how the receive call will function. If the
// timeout is less than 0, then it will block for ever. If it is set 0, then
// it will return immediately. If the caller wishes there to be a timeout, then
// the timeout should be specified in milliseconds.
//
// For the no wait and timeout modes, if no message is received then both return
// values will be nil.
func (service *ServiceRequester) ReceiveResponse(timeout int) (*types.MsgEnvelope, error) {
	if service.client.IsClosed() {
		return nil, errors.New("Message bus context has been destroyed")
	}
	if service.servCtx == nil {
		return nil, errors.New("Service already closed")
	}

	var msg *types.MsgEnvelope
	var err error

	if timeout < 0 {
		msg, err = service.client.ctx.ReceiveWait(service.servCtx)
	} else if timeout == 0 {
		msg, err = service.client.ctx.ReceiveNoWait(service.servCtx)
	} else {
		msg, err = service.client.ctx.ReceiveTimedWait(service.servCtx, timeout)
	}

	return msg, err
}

// Send a request to the service.
func (service *ServiceRequester) Request(request interface{}) error {
	if service.client.IsClosed() {
		return errors.New("Message bus context has been destroyed")
	}
	if service.servCtx == nil {
		return errors.New("Service already closed")
	}
	return service.client.ctx.Request(service.servCtx, request)
}

// Close the subscriber
func (service *ServiceRequester) Close() {
	if service.servCtx == nil {
		return
	}

	service.client.ctx.DestroyRecvCtx(service.servCtx)
	service.servCtx = nil
}
