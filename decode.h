/* Copyright (c) 2011 Akamai Technologies, Inc. */

// 0 + EINVAL + EIO
int uri_decode(const StringPiece& sp, ostream& os);
int b64_decode(const StringPiece& sp, ostream& os);
