package org.mangler;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.ProgressBar;
import android.view.View;
import android.content.Intent;
import android.os.Handler;

public class Login extends Activity {
	
	final private Handler ManglerHandler = new Handler();
	
	private void showError(final String error) {
		ManglerHandler.post(new Runnable() {
			public void run() {
				hideProgress();
				((TextView)findViewById(R.id.errors)).setText(error);
				((TextView)findViewById(R.id.errors)).setVisibility(View.VISIBLE);
			}
		});
	}
	
	private void hideError() {
		ManglerHandler.post(new Runnable() {
			public void run() {			
				((TextView)findViewById(R.id.errors)).setVisibility(View.GONE);
			}
		});
	}
	
	private void showProgress() {
		ManglerHandler.post(new Runnable() {
			public void run() {		
				hideError();
				((ProgressBar)findViewById(R.id.progress)).setVisibility(View.VISIBLE);
			}
		});
	}
	
	private void hideProgress() {
		ManglerHandler.post(new Runnable() {
			public void run() {			
				((ProgressBar)findViewById(R.id.progress)).setVisibility(View.GONE);
			}
		});
	}
	
	private void disableViews() {
		ManglerHandler.post(new Runnable() {
			public void run() {
				((EditText)findViewById(R.id.server)).setEnabled(false);
				((EditText)findViewById(R.id.username)).setEnabled(false);
				((EditText)findViewById(R.id.password)).setEnabled(false);
				((Button)findViewById(R.id.connect)).setEnabled(false);
			}
		});
	}
	
	private void enableViews() {
		ManglerHandler.post(new Runnable() {
			public void run() {
				((EditText)findViewById(R.id.server)).setEnabled(true);
				((EditText)findViewById(R.id.username)).setEnabled(true);
				((EditText)findViewById(R.id.password)).setEnabled(true);
				((Button)findViewById(R.id.connect)).setEnabled(true);
			}
		});
	}
	
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
		final Intent ManglerIntent = new Intent(Login.this, Mangler.class);
		
		System.loadLibrary("ventrilo3");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login);
        
        if(VentriloInterface.isloggedin() != 0) 
        {
        	Login.this.startActivity(ManglerIntent);
        }
        
        // Check for click events.
        final Button connect = (Button)findViewById(R.id.connect);
        connect.setOnClickListener(new View.OnClickListener() 
        {
        	public void onClick(View v) 
        	{	
        		new Thread(new Runnable() 
        		{
        			public void run() 
        			{
        				disableViews();
        				showProgress();
        				
                		String server   = ((EditText)findViewById(R.id.server)).getText().toString();
                		String username = ((EditText)findViewById(R.id.username)).getText().toString();
                		String password = ((EditText)findViewById(R.id.password)).getText().toString();
                		
                		if(VentriloInterface.isloggedin() == 0)
                		{
                			if(!server.equals("") && !username.equals("")) 
                			{
                				if(VentriloInterface.login(server, username, password, "") != 0) 
                				{
                					Login.this.startActivity(ManglerIntent);
                				}
                				else 
                				{
                					showError("Could not connect to server.");
                					enableViews();
                				}
                			}
                			else 
                			{
                				showError("Server or username were left empty.");
                				enableViews();
                			}
                		}
                		else 
                		{
                			Login.this.startActivity(ManglerIntent);
                		}
        			}
        		}).start();
        	}
        });
    }
}