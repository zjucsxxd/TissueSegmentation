/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkVectorConfidenceConnectedImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRGBPixel.h"

int main( int argc, char *argv[] )
{
  if( argc < 8 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " inputImage outputImage inputSeedsFile multiplier iterations radius outputMembershipFunctionFile" << std::endl;
    return EXIT_FAILURE;
    }

  typedef   unsigned char                         PixelComponentType;
  typedef   itk::RGBPixel< PixelComponentType >   InputPixelType;
  const     unsigned int    Dimension = 2;

  typedef itk::Image< InputPixelType, Dimension >  InputImageType;

  typedef unsigned char                            OutputPixelType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

  typedef  itk::ImageFileReader<  InputImageType   > ReaderType;
  typedef  itk::ImageFileWriter<  OutputImageType  > WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( argv[1] );
  writer->SetFileName( argv[2] );

  typedef  itk::VectorConfidenceConnectedImageFilter< InputImageType,
                                    OutputImageType > ConnectedFilterType;

  ConnectedFilterType::Pointer confidenceConnected = ConnectedFilterType::New();

  confidenceConnected->SetInput( reader->GetOutput() );
  writer->SetInput( confidenceConnected->GetOutput() );

  const double multiplier = atof( argv[4] );

  confidenceConnected->SetMultiplier( multiplier );

  const unsigned int iterations = atoi( argv[5] );

  confidenceConnected->SetNumberOfIterations( iterations );

  confidenceConnected->SetReplaceValue( 255 );

  try
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  InputImageType::ConstPointer inputImage = reader->GetOutput();

  InputImageType::IndexType            seedIndex;

  double seedX;
  double seedY;

  std::ifstream inputSeedsFile;

  inputSeedsFile.open( argv[3] );

  while( inputSeedsFile >> seedX >> seedY )
    {
    seedIndex[0] = static_cast< InputImageType::IndexValueType >( seedX );
    seedIndex[1] = static_cast< InputImageType::IndexValueType >( seedY );
    confidenceConnected->AddSeed( seedIndex );
    }

  inputSeedsFile.close();

  const unsigned int neighborhoodRadius = atoi( argv[6] );

  confidenceConnected->SetInitialNeighborhoodRadius( neighborhoodRadius );

  std::cout << "multiplier = " << multiplier << std::endl;
  std::cout << "iterations = " << iterations << std::endl;
  std::cout << "neighborhoodRadius = " << neighborhoodRadius << std::endl;

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  typedef ConnectedFilterType::MeanVectorType   MeanVectorType;

  const MeanVectorType & mean = confidenceConnected->GetMean();

  std::cout << "Mean vector = " << std::endl;
  std::cout << mean << std::endl;

  typedef ConnectedFilterType::CovarianceMatrixType   CovarianceMatrixType;

  const CovarianceMatrixType & covariance = confidenceConnected->GetCovariance();

  std::cout << "Covariance matrix = " << std::endl;
  std::cout << covariance << std::endl;

  std::ofstream outputMembership;

  outputMembership.open( argv[7] );

  for( unsigned int i = 0; i < 3; i++ )
    {
    outputMembership << mean[i] << '\t';
    }

  outputMembership << std::endl;

  for( unsigned int i = 0; i < 3; i++ )
    {
    for( unsigned int j = 0; j < 3; j++ )
      {
      outputMembership << covariance(i,j) << '\t';
      }
    outputMembership << std::endl;
    }

  outputMembership.close();

  return EXIT_SUCCESS;
}
