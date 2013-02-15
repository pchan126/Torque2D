<?php

require_once('database.php');

define('SUCCESS', '0');
define('ERROR_VERIFICATION_NO_RESPONSE', '1');
define('ERROR_VERIFICATION_FAILED', '2');
define('ERROR_INVALID_PRODUCT_ID', '3');
define('ERROR_TRANSACTION_ALREADY_PROCESSED', '4');

define('PRODUCT_ID_PREFIX', 'xxxxxxxxxxxxx');

function beginsWith($str, $sub) {
	return (strncmp($str, $sub, strlen($sub)) == 0);
	}

function result($status, $error = '') {
  return array(
  				'status' => $status,
  				'error' => $error
  				);
  				}
  				
function logToFile($msg) {
	$fd = fopen('log.txt', 'a');
	$str = '[' . date('Y/m/d h:i:s', mktime()).']'.$msg;
	fwrite($fd, $str . "\r\n");
	fclose($fd);
	}
	
function validateReceipt($receipt, $sandbox)
{
    if ($sandbox) {
	    $store = 'https://sandbox.itunes.apple.com/verifyReceipt';
    } else {
	    $store = 'https://buy.itunes.apple.com/verifyReceipt';
    }

	$postData = json_encode(array('receipt-data' => $receipt));
	
	$ch = curl_init($store);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_POST, true);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $postData);
	$encodedResponse = curl_exec($ch);
	curl_close($ch);
	
	if ($encodedResponse == false)
	{
		return result(ERROR_VERIFICATION_NO_RESPONSE,
		'Payment could not be verified (no response data).');
	}	
	$response = json_decode($encodedResponse);
	$status = $response->{'status'};
	$decodedReceipt = $response->{'receipt'};
	if ($status != 0)
	{ 
		return result(ERROR_VERIFICATION_FAILED, "Payment could not be verified (status = $status).");
	}
	
	logToFile(print_r($decodedReceipt, true));
	
 // 1
    $product_id = $decodedReceipt->{'product_id'};
    $transaction_id = $decodedReceipt->{'transaction_id'};
    $original_transaction_id = $decodedReceipt->{'original_transaction_id'};
    
    //2
    if (!beginsWith($product_id, PRODUCT_ID_PREFIX)) {
    	return result(ERROR_INVALID_PRODUCT_ID, 'Invalid product id.');
    	}
    	
    //3
	$db = Database::get();
	$statement = $db->prepare('SELECT * FROM transactions WHERE transaction_id=?');
	$statement->bindParam(1, $transaction_id, PDO::PARAM_STR, 32);
	$statement->execute();
	
	//4
	if ($statement->rowCount() > 0) {
		logToFile("Already processed $transaction_id.");
		return result(ERROR_TRANSACTION_ALREADY_PROCESSED,
			'Already processed this transaction.');
			}
	
	//5
	else {
		logToFile("Adding $transaction_id.");
		$statement = $db->prepare(
			'INSERT INTO transactions(transaction_id, product_id, original_transaction_id) VALUES (?, ?, ?)');
		$statement->bindParam(1, $transaction_id, PDO::PARAM_STR, 32);
		$statement->bindParam(2, $product_id, PDO::PARAM_STR, 32);
		$statement->bindParam(3, $original_transaction_id, PDO::PARAM_STR, 32);
		$statement->execute();
		}

	return result(SUCCESS);
	
}
$receipt = $_POST['receipt'];
$sandbox = $_POST['sandbox'];
$retval = validateReceipt($receipt, $sandbox);

header('content-type: application/json; charset=uft-8');
echo json_encode($retval);

?>