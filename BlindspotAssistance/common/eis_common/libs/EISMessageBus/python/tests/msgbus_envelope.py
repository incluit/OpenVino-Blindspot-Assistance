"""Unit tests for EIS MsgBus Message Envelope
"""
import unittest
from eis.msg_envelope import MsgEnvelope


class TestMsgEnvelope(unittest.TestCase):
    """MsgEnvelope object unit tests.
    """
    def test_metadata(self):
        """Test MsgEnvelope when only meta_data is set.
        """
        msg = MsgEnvelope(meta_data={'test': 'test'})
        data, _ = msg
        self.assertEqual(data['test'], 'test')

    def test_blob(self):
        """Test MsgEnvelope when only blob is set.
        """
        msg = MsgEnvelope(blob=b'\x00\x01\x02')
        _, blob = msg
        self.assertEqual(blob, b'\x00\x01\x02')

    def test_metadata_blob(self):
        """Test MsgEnvelope with both meta_data and blob.
        """
        msg = MsgEnvelope(meta_data={'test': 'test'}, blob=b'\x00\x01\x02')
        other = MsgEnvelope(meta_data={'test': 'test'}, blob=b'\x00\x01\x02')

        # Verify tuple unpacking
        meta_data, blob = msg
        self.assertEqual(meta_data, {'test': 'test'})
        self.assertEqual(blob, b'\x00\x01\x02')

        # Verify using msg as a tuple works
        self.assertEqual(msg[0], {'test': 'test'})
        self.assertEqual(msg[1], b'\x00\x01\x02')
        self.assertEqual(msg, ({'test': 'test'}, b'\x00\x01\x02',))

        # Verify comparison of MsgEnvelope objects works
        self.assertEqual(msg, other)

    def test_invalid_idx(self):
        """Test MsgEnvelope accessing bad tuple idx
        """
        msg = MsgEnvelope(meta_data={'test': 'test'}, blob=b'\x00\x01\x02')
        with self.assertRaises(IndexError):
            i = msg[2]

    def test_bytes(self):
        """Test MsgEnvelope bytes conversion.
        """
        msg = MsgEnvelope(meta_data={'test': 'test'}, blob=b'\x00\x01\x02')
        with self.assertRaises(TypeError):
            b = bytes(msg)

        msg = MsgEnvelope(meta_data={'test': 'test'})
        with self.assertRaises(TypeError):
            b = bytes(msg)

        msg = MsgEnvelope(blob=b'\x00\x01\x02')
        b = bytes(msg)


if __name__ == '__main__':
    unittest.main()
