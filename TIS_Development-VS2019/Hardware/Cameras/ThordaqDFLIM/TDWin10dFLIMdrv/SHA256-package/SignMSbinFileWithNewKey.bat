REM Command to sign Microsoft test file ("SignableFile.bin") for partner center upload
REM First, download a MS Partner "signable file" from web, then sign it, then upload it to MS Partner website
REM Second, Microsoft should recognize the certificate used to sign the "signable file" when we use
REM that certificate to sign our kernel driver

signtool sign /a /sha1 524e5ae5e2d337b7b8bea021bb0bb311e08a13dc /fd SHA256 C:\temp\SignableFile.bin   