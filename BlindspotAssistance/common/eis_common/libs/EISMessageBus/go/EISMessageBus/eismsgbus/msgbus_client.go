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
	"sync"
)

// Message bus client object
type MsgbusClient struct {
	ctx    *msgbus.MsgbusContext
	mu     sync.Mutex
	closed bool
}

// Initialize a new message bus context.
func NewMsgbusClient(config map[string]interface{}) (*MsgbusClient, error) {
	msgbusCtx, err := msgbus.NewMsgbusContext(config)
	if err != nil {
		return nil, err
	}

	ctx := new(MsgbusClient)
	ctx.ctx = msgbusCtx
	ctx.closed = false

	return ctx, nil
}

// Create a new publisher on the message bus context.
func (ctx *MsgbusClient) NewPublisher(topic string) (*Publisher, error) {
	ctx.mu.Lock()
	defer ctx.mu.Unlock() // This may not need to be thread-safe
	if ctx.closed {
		return nil, errors.New("Message bus context already destroyed")
	}

	pubCtx, err := ctx.ctx.NewPublisher(topic)
	if err != nil {
		return nil, err
	}

	publisher := newPublisher(ctx, pubCtx)

	return publisher, nil
}

// Create a new subscriber for the specified topic.
func (ctx *MsgbusClient) NewSubscriber(topic string) (*Subscriber, error) {
	msgCh := make(chan *types.MsgEnvelope)
	errCh := make(chan error)
	quitCh := make(chan interface{})
	subCtx, err := ctx.ctx.NewSubscriber(topic)
	if err != nil {
		close(msgCh)
		close(errCh)
		close(quitCh)
		return nil, err
	}

	// Create receive routine structure
	recv := receiveRoutine{
		RecvCtx:   subCtx,
		MsgCh:     msgCh,
		ErrCh:     errCh,
		QuitCh:    quitCh,
		MsgbusCtx: ctx,
	}

	// Start goroutine to receive publications
	go recv.run()

	// Create subscriber and return
	return newSubscriber(msgCh, quitCh, errCh), nil
}

// Create a new service to receive requests over and send responses from.
func (ctx *MsgbusClient) NewService(serviceName string) (*Service, error) {
	servCtx, err := ctx.ctx.NewService(serviceName)
	if err != nil {
		return nil, err
	}

	return newService(ctx, servCtx), nil
}

func (ctx *MsgbusClient) GetService(serviceName string) (*ServiceRequester, error) {
	servCtx, err := ctx.ctx.GetService(serviceName)
	if err != nil {
		return nil, err
	}

	return newServiceRequester(ctx, servCtx), nil
}

// Close the messaage bus client.
//
// **IMPORTANT NOTE:** After this is called, no other operations using this context
// will succeed (including methods on publishers, subscribers, etc.).
func (ctx *MsgbusClient) Close() {
	ctx.mu.Lock()
	defer ctx.mu.Unlock()
	if ctx.closed {
		return
	}
	ctx.ctx.Destroy()
	ctx.ctx = nil
	ctx.closed = true
}

// Check if the message bus client has already been destroyed.
func (ctx *MsgbusClient) IsClosed() bool {
	ctx.mu.Lock()
	defer ctx.mu.Unlock()
	return ctx.closed
}

// Receive routine structure
type receiveRoutine struct {
	RecvCtx   *msgbus.ReceiveContext
	MsgCh     chan *types.MsgEnvelope
	ErrCh     chan error
	QuitCh    chan interface{}
	MsgbusCtx *MsgbusClient
}

// Goroutine method for receiving messages and sending them over the given channel
func (recv *receiveRoutine) run() {
	ctx := recv.MsgbusCtx.ctx

	// When the method exists destroy the receive context
	defer ctx.DestroyRecvCtx(recv.RecvCtx)

	// Infinite loop while receiving messages until told to quit over the QuitCh
	for {
		msg, err := ctx.ReceiveWait(recv.RecvCtx)
		if err != nil {
			recv.ErrCh <- err
		}

		// If the quit channel has been closed, then return from the routine method
		select {
		case <-recv.QuitCh:
			return
		default:
		}

		// Only send the message over the receive channel if no error was received
		// prior to this point
		if err == nil {
			recv.MsgCh <- msg
		}
	}
}
