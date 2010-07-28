package org.mangler.android;

import android.content.Context;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class VolumePreference extends DialogPreference implements OnSeekBarChangeListener {
	
	int level = 79;
	
    public VolumePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setupLayout(context, attrs);
    }

    public VolumePreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setupLayout(context, attrs);
    }

    private void setupLayout(Context context, AttributeSet attrs) { }

    @Override
    protected View onCreateDialogView() {
        SeekBar seek = new SeekBar(getContext());
        
        seek.setMax(158);
        
        level = getPersistedInt(79);
        
        seek.setProgress(level);
        
        seek.setPadding(10, 10, 10, 10);
        seek.setOnSeekBarChangeListener(this);

        return seek;
    }

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
    	if (progress >= 67 && progress <= 81) {
			seekBar.setProgress(74);
		} else if (progress >= 148) {
			seekBar.setProgress(158);
		}
    	level = seekBar.getProgress();
    }
    
    public void onDialogClosed(boolean positiveResult) {
    	if (positiveResult) {
    		persistInt(level);
    	}
    }

    public void onStartTrackingTouch(SeekBar seek) { }

    public void onStopTrackingTouch(SeekBar seek) { }
}
