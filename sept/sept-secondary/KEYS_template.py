NUM_KEYS = 2

HOVI_ENC_KEY_PRD = [
    bytearray.fromhex('00000000000000000000000000000000'),
    bytearray.fromhex('00000000000000000000000000000000'),
]

HOVI_SIG_KEY_PRD  = [
    bytearray.fromhex('00000000000000000000000000000000'),
    bytearray.fromhex('00000000000000000000000000000000'),
]

IV = [
    bytearray.fromhex('00000000000000000000000000000000'),
    bytearray.fromhex('00000000000000000000000000000000'),
]

assert len(HOVI_ENC_KEY_PRD) == NUM_KEYS
assert len(HOVI_SIG_KEY_PRD) == NUM_KEYS
assert len(IV) == NUM_KEYS