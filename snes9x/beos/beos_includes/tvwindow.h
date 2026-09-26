{
	static uint32 i = 0;
	static clipping_rect *clip;
	m_locker->Lock();
	if(m_connected) {
		clip = m_cliplist;
		for(i = 0; i < m_numcliprects; i++, clip++) {
			int32 width = (clip->right - clip->left)+1;
			int32 height = (clip->bottom - clip->top)+1;
			if(width == 512 && height == 478)
			TvMode((uint8 *)off_screen->Bits(), off_screen->BytesPerRow(), (uint8 *)Delta, m_bits + clip->left * m_bitsperpixel, m_rowbytes, width >> 1, height >> 1);
		}
	}
	m_locker->Unlock();
}